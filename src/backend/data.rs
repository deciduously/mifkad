// data.rs handles reading in the Enrollment: Site report and populating internal data structure

// TODO Xls OR Xlsx
use super::{schema, DATAFILE};
use calamine::{open_workbook, Reader, Xls};
use errors::{Result, ResultExt};
use regex::Regex;
use schema::{Classroom, Expected, Kid, School};

// scrape enrollment will read in the Enrollment excel sheet and populate the School
// TODO parameterize the sheet location
pub fn scrape_enrollment(day: schema::Weekday, file_str: &str) -> Result<School> {
    lazy_static! {
        // Define patterns to match
        static ref KID_RE: Regex =
            Regex::new(r"((@|#|&) )?(?P<last>[A-Z]+), (?P<first>[A-Z]+)").unwrap();
        static ref CLASS_RE: Regex = Regex::new(r"CLASSROOM: ([A-Z])").unwrap();
        static ref CAPACITY_RE: Regex = Regex::new(r"CLASS MAXIMUM: (\d+)").unwrap();
    }

    info!("Loading {:?} from {}", day, DATAFILE);
    let mut school = School::new(&day);

    // Use calamine to read in the input sheet
    let mut excel: Xls<_> = open_workbook(file_str).unwrap();

    let mut headcount = 0;

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
                        if school.classrooms.len() > 0 {
                            let last_class = school.classrooms[school.classrooms.len() - 1].clone();
                            let prev_headcount = last_class.kids.len();
                            headcount += prev_headcount;
                            info!("Room {} headcount: {}", last_class.letter, prev_headcount,);
                        }

                        // create a new Classroom and push it to the school
                        let new_class = Classroom::new(caps[1].to_string(), capacity);
                        debug!(
                            "FOUND CLASS: {} (max {})",
                            &new_class.letter, &new_class.capacity
                        );
                        school.classrooms.push(new_class);
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
                        let mut new_kid = Kid::new(name, day, &format!("{}", sched));
                        debug!(
                            "FOUND KID: {} - {} ({:?})",
                            new_kid.name, sched, new_kid.schedule.expected
                        );
                        // If the kid is scheduled, push the kid to the latest open class
                        if new_kid.schedule.expected == Expected::Unscheduled {
                            info!(
                                "{} not scheduled on {:?} - omitting from roster",
                                &new_kid.name, day
                            );
                        } else {
                            let mut classroom = school.classrooms.pop().expect(
                                "Kid found before classroom declaration - input file malformed",
                            );
                            classroom.push_kid(new_kid);
                            school.classrooms.push(classroom);
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
    info!(
        "ENROLLMENT LOADED - {:?} - total headcount {}",
        day, headcount
    );

    Ok(school)
}

#[cfg(test)]
mod tests {
    use super::{schema, scrape_enrollment};
    use std::str::FromStr;

    #[test]
    fn test_open_excel() {
        let school =
            scrape_enrollment(schema::Weekday::from_str("mon").unwrap(), "sample/test.xls")
                .unwrap();
        assert!(school.classrooms.len() > 0)
    }
}
