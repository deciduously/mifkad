// data.rs handles reading in the Enrollment: Site report and populating internal data structure

// TODO Xls OR Xlsx
use calamine::{open_workbook, Reader, Xls};
use chrono::prelude::{Date, Datelike, Local};
use config::Config;
use errors::{Result, ResultExt};
use regex::Regex;
use schema::{self, Classroom, Expected, ExtendedDayConfig, Kid, School, Weekday};
use serde_json;
use std::{
    fs::{create_dir, remove_file, File, OpenOptions},
    io::{prelude::Write, BufWriter},
    path::PathBuf,
    str::FromStr,
};
use util::*;

lazy_static! {
    // this lazy_static block facilitates the database file setup based on the current localtime system date at runtime

    // Establish date
    static ref TODAY: Date<Local> = Local::today();
    // note - this is a schema::Weekday, not a chrono::Weekday
    static ref WEEKDAY: Weekday = Weekday::from_str(&format!("{:?}", TODAY.weekday())).expect("Could not get a weekday from chrono");
    static ref DAY_STR: String = format!("{}{}{}", TODAY.year(), TODAY.month(), TODAY.day());

    // Paths are unlikely to change, so I'm hardcoding them.
    static ref DB_FILE: PathBuf = {
        let mut ret = PathBuf::new();
        let mut s = String::from_str(&DAY_STR).unwrap();
        s.push_str(".json");
        ret.push(&s);
        ret
    };
    static ref DB_FILE_STR: &'static str = DB_FILE.to_str().unwrap();
    static ref DB_DIR: PathBuf = {
        let mut ret = PathBuf::new();
        ret.push("mifkad-assets");
        ret.push("db");
        ret
    };
    static ref DB_DIR_STR: &'static str = DB_DIR.to_str().unwrap();
    static ref DB_FILEPATH: PathBuf = {
        let mut ret = PathBuf::new();
        ret.push(DB_DIR.to_str().unwrap());
        ret.push(DB_FILE.to_str().unwrap());
        ret
    };
    static ref DB_FILEPATH_STR: &'static str = DB_FILEPATH.to_str().unwrap();
}

// Determine what day it is, and either write a new db file or read the one there
// It returns the school to load in to the AppState
pub fn init_db(config: &Config) -> Result<(School)> {
    // Kid(id,name,classroom,date,expected,actual) might be a good idea if I ever go sql
    // To update, we'll select for Name AND Day, or pass the ID of the record down to the frontend

    // Open up our db folder in mifkad-assets.  If it doesnt exist, create it
    if !DB_DIR.exists() {
        warn!("No db found!  Creating...");
        create_dir(*DB_DIR_STR).chain_err(|| "Could not create mifkad-assets\\db")?;
    }

    // make sure we've got our extended day stuff good to go
    let extended_config = get_extday(&config)?;

    // Now, check if we have an entry for today.  If it doesn't exist, write it from the GAN data

    let ret = if let Ok(fs_db) = File::open(*DB_FILEPATH_STR) {
        info!("Using existing db file {}", *DB_FILE_STR);
        let school: schema::School = serde_json::from_str(&string_of_file(&fs_db)?)
            .chain_err(|| format!("Malformed {}", *DB_FILE_STR))?;
        school
    } else {
        warn!(
            "No record found for {}.  Reading {:?}",
            *DAY_STR, config.roster
        );
        let school = scrape_enrollment(*WEEKDAY, extended_config, &config)?;
        let mut new_f = File::create(*DB_FILEPATH_STR)
            .chain_err(|| format!("could not create {}", *DB_FILE_STR))?;
        new_f
            .write_all(json!(school).to_string().as_bytes())
            .chain_err(|| format!("could not write data to {}", *DB_FILE_STR))?;
        school
    };

    warn!(
        "Successfully initialized Mifkad using mifkad-assets\\db\\{}",
        *DB_FILE_STR
    );
    warn!("{}", config);
    warn!("Starting webserver...");
    Ok(ret)
}

pub fn get_extday(config: &Config) -> Result<ExtendedDayConfig> {
    if let Ok(fs_extconf) = File::open(&config.extended_config) {
        // we have it already, return it
        info!("Found extended day config, loading...");
        Ok(serde_json::from_str(&string_of_file(&fs_extconf)?)
            .chain_err(|| format!("Malformed json in {:?}", config.extended_config))?)
    } else {
        // create new file with the default value, then return it
        warn!("No extended day config file found, creating default...");
        let mut new_f = File::create(&config.extended_config)
            .chain_err(|| format!("Could not create {:?}", config.extended_config))?;
        let ret = ExtendedDayConfig::default();
        new_f
            .write_all(json!(ret).to_string().as_bytes())
            .chain_err(|| format!("Could not write to {:?}", config.extended_config))?;
        Ok(ret)
    }
}

pub fn reset_extday(new_config: &ExtendedDayConfig, config: &Config) -> Result<()> {
    // replace whatever's there with the incoming, creating it if it doens't exist
    let mut fs_extconf = OpenOptions::new()
        .write(true)
        .truncate(true)
        .create(true)
        .open(&config.extended_config)
        .chain_err(|| format!("Could not open {:?}", &config.extended_config))?;
    fs_extconf
        .write_all(json!(new_config).to_string().as_bytes())
        .chain_err(|| {
            format!(
                "Could not write new extended day config to {:?}",
                config.extended_config
            )
        })?;
    Ok(())
}

pub fn reset_db(config: &Config) -> Result<()> {
    // Delete current file and replace it with a brand new copy
    remove_file(*DB_FILEPATH_STR).chain_err(|| format!("Could not clear {}", *DB_FILE_STR))?;
    let mut new_db = File::create(*DB_FILEPATH_STR)
        .chain_err(|| format!("Could not create {}", *DB_FILE_STR))?;
    new_db
        .write_all(
            serde_json::to_string(&scrape_enrollment(*WEEKDAY, get_extday(&config)?, &config)?)
                .chain_err(|| "Could not serialize school")?
                .as_bytes(),
        )
        .chain_err(|| format!("Could not write data to {}", *DB_FILE_STR))?;
    Ok(())
}

// scrape enrollment will read in the Enrollment excel sheet and populate the School
// TODO parameterize the sheet location
pub fn scrape_enrollment(
    day: Weekday,
    extended_config: ExtendedDayConfig,
    config: &Config,
) -> Result<School> {
    lazy_static! {
        // Define patterns to match
        static ref KID_RE: Regex =
            Regex::new(r"((@|#|&) )?(?P<last>[A-Z]+), (?P<first>[A-Z]+)").unwrap();
        static ref CLASS_RE: Regex = Regex::new(r"CLASSROOM: ([A-Z])").unwrap();
        static ref CAPACITY_RE: Regex = Regex::new(r"CLASS MAXIMUM: (\d+)").unwrap();
    }

    info!("Loading {:?} from {:?}", day, &config.roster);
    let mut school = School::new(day, extended_config);

    // Use calamine to read in the input sheet
    let mut excel: Xls<_> = open_workbook(&config.roster).unwrap();

    let mut headcount = 0;
    let mut classcount = 0;

    // Try to get "Sheet1" as `r` - it should always exist
    if let Some(Ok(r)) = excel.worksheet_range("Sheet1") {
        // Process each row
        for row in r.rows() {
            use calamine::DataType::*;
            // Column A is either a Class or a Kid
            let column_a = &row[0];
            match column_a {
                String(s) => {
                    // If it's a class, open up a new class
                    // If its a kid, push it to the open class
                    // If it's anything else, ignore it.
                    if CLASS_RE.is_match(&s) {
                        debug!("MATCH CLASS: {}", &s);
                        let caps = CLASS_RE.captures(&s).unwrap();
                        // the capacity is found in Column B
                        let mut capacity: u8;
                        match &row[1] {
                            String(s2) => {
                                let capacity_caps = CAPACITY_RE.captures(&s2).unwrap();
                                capacity = (&capacity_caps[1])
                                    .parse::<u8>()
                                    .chain_err(|| "Unable to parse capacity as u8")?;
                            }
                            _ => {
                                bail!("Column B of Classroom declaration contained unexpected data")
                            }
                        }

                        // Display the previous class headcount  -this needs to happen once againa the end, and not the first time
                        if !school.classrooms.is_empty() {
                            let last_class = school.classrooms[school.classrooms.len() - 1].clone();
                            let prev_headcount = last_class.kids.len();
                            debug!("Room {} headcount: {}", last_class.letter, prev_headcount);
                        }

                        // create a new Classroom and push it to the school
                        let new_class = Classroom::new(classcount, caps[1].to_string(), capacity);
                        debug!(
                            "FOUND CLASS: {} (max {})",
                            &new_class.letter, &new_class.capacity
                        );
                        school.classrooms.push(new_class);
                        classcount += 1;
                    } else if KID_RE.is_match(&s) {
                        let caps = KID_RE.captures(&s).unwrap();

                        // Reformat name from LAST, FIRST to FIRST LAST
                        let mut name = ::std::string::String::from(&caps["first"]);
                        name.push_str(" ");
                        name.push_str(&caps["last"]);

                        // init Kid datatype

                        // Add schedule day
                        let sched_idx = match day {
                            schema::Weekday::Monday => 6,
                            schema::Weekday::Tuesday => 7,
                            schema::Weekday::Wednesday => 8,
                            schema::Weekday::Thursday => 9,
                            schema::Weekday::Friday => 10,
                        };
                        let sched = &row[sched_idx];
                        let mut new_kid = Kid::new(headcount, name, &format!("{}", sched));
                        debug!(
                            "FOUND KID: {} - {} ({:?})",
                            new_kid.name, sched, new_kid.schedule.expected
                        );
                        // If the kid is scheduled, push the kid to the latest open class
                        if new_kid.schedule.expected == Expected::Unscheduled {
                            debug!(
                                "{} not scheduled on {:?} - omitting from roster",
                                &new_kid.name, day
                            );
                        } else {
                            let mut classroom = school.classrooms.pop().expect(
                                "Kid found before classroom declaration - input file malformed",
                            );
                            classroom.push_kid(new_kid);
                            school.classrooms.push(classroom);
                            headcount += 1;
                            debug!("Adding to response");
                        }
                    }
                }
                _ => continue,
            }
        }
    }

    // Print out the status info
    let last_class = school.classrooms[school.classrooms.len() - 1].clone();
    info!(
        "Room {} headcount: {}",
        last_class.letter,
        last_class.kids.len(),
    );
    warn!(
        "Successfully loaded {:?} enrollment from {:?} - total headcount {}, total classcount {}",
        day, config.roster, headcount, classcount
    );

    Ok(school)
}

pub fn write_db(school: &School) -> Result<()> {
    // Open as writable - will overwrite contents
    let f = OpenOptions::new()
        .write(true)
        .truncate(true)
        .open(*DB_FILEPATH_STR)
        .chain_err(|| "Could not open db file")?;
    let out = serde_json::to_string(school).chain_err(|| "Could not serialize data structure")?;
    let mut bfw = BufWriter::new(f);
    bfw.write(out.as_bytes())
        .chain_err(|| "Could not write out data structure")?;
    Ok(())
}
