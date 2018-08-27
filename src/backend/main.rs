extern crate actix;
extern crate actix_web;
extern crate calamine;
extern crate chrono;

#[macro_use]
extern crate error_chain;
#[macro_use]
extern crate lazy_static;
#[macro_use]
extern crate log;
#[cfg(test)]
extern crate pretty_assertions;
extern crate pretty_env_logger;
extern crate regex;
extern crate serde;
#[macro_use]
extern crate serde_derive;
#[macro_use]
extern crate serde_json;
//extern crate uuid;

mod data;
mod errors {
    error_chain!{}
}
mod handlers;
mod schema;

use actix_web::{
    fs::StaticFiles,
    http,
    middleware::{self, cors::Cors},
    server::HttpServer,
    App,
};
use chrono::prelude::{Date, Datelike, Local};
use errors::{Result, ResultExt};
use handlers::{index, school, school_today};
use std::{
    cell::Cell,
    env::{set_var, var},
    fs::{create_dir, read_dir, File},
    io::{prelude::*, BufReader},
    path::{Path, PathBuf},
    str::FromStr,
};

static DATAFILE: &str = "current.xls";

lazy_static! {
    // lazy_static facilitates the database file setup based on the current localtime system date at runtime

    // Establish date
    static ref TODAY: Date<Local> = Local::today();
    // schema::Weekday only will ever be Mon-Fri, as opposed to chrono::Weekday
    static ref WEEKDAY: schema::Weekday = schema::Weekday::from_str(&format!("{:?}", TODAY.weekday())).expect("Could not get a weekday from chrono");
    static ref DAY_STR: String = format!("{}{}{}", TODAY.year(), TODAY.month(), TODAY.day());
    // Paths are unlikely to change, so I'm hardcoding them.
    static ref DB_FILE: PathBuf = {
        let mut ret = PathBuf::new();
        let mut s = String::from_str(&DAY_STR).unwrap();
        s.push_str(".json");
        ret.push(&s);
        ret
    };
    static ref DB_DIR: PathBuf = {
        let mut ret = PathBuf::new();
        ret.push("mifkad-assets");
        ret.push("db");
        ret
    };
    static ref DB_FILEPATH: PathBuf = {
        let mut ret = PathBuf::new();
        ret.push(DB_DIR.to_str().unwrap());
        ret.push(DB_FILE.to_str().unwrap());
        ret
    };
}

struct AppState {
    school: Cell<schema::School>,
}

// Determine what day it is, and either write a new db file or read the one there
// It returns the school to load in to the AppState
fn init_db() -> Result<(schema::School)> {
    // Open up our db folder in mifkad-assets.  If it doesnt exist, create it

    if !DB_DIR.exists() {
        warn!("No db found!  Creating...");
        create_dir(DB_DIR.to_str().unwrap()).chain_err(|| "Could not create mifkad-assets\\db")?;
    }

    // Now, check if we have an entry for today.  If it doesn't exist, write it from the GAN data
    // First, get the contents of the directory
    let dir_listing: Vec<PathBuf> = read_dir(DB_DIR.to_str().unwrap())
        .chain_err(|| "could not read db!")?
        .map(|f| f.expect("could not read db entry").path())
        .collect();

    // Try to locate today.
    let mut found = false;
    for l in &dir_listing {
        let curr_str = l.to_str().unwrap();
        if curr_str == DB_FILEPATH.to_str().unwrap() {
            info!(
                "Found previously logged attendance for {}, loading...",
                *DAY_STR
            );
            found = true;
            break;
        }
    }

    // If we didn't find a corresponding file, serialize it out from DATAFILE
    if !found {
        info!("No record found for {}.  Reading {}", *DAY_STR, DATAFILE);
        let school = json!(data::scrape_enrollment(*WEEKDAY, DATAFILE)?);
        let mut new_f = File::create(DB_FILEPATH.to_str().unwrap())
            .chain_err(|| format!("could not create {}", DB_FILE.to_str().unwrap()))?;
        new_f
            .write_all(school.to_string().as_bytes())
            .chain_err(|| format!("could not write data to {}", DB_FILE.to_str().unwrap()))?;
    }

    // Finally, read in today's entry from the database, which we've ensured exists
    let f = File::open(Path::new(DB_FILEPATH.to_str().unwrap()))
        .chain_err(|| format!("could not open {}", DB_FILE.to_str().unwrap()))?;
    let mut bfr = BufReader::new(f);
    let mut input_str = String::new();
    let _ = bfr.read_to_string(&mut input_str);

    // Deserialize the file contents and return
    let ret: schema::School = serde_json::from_str(&input_str)
        .chain_err(|| format!("could not parse contents of {}", DB_FILE.to_str().unwrap()))?;

    info!(
        "Mifkad initialized - using file {}",
        DB_FILE.to_str().unwrap()
    );
    Ok(ret)
}

// Start env_logger - for now, change this number to change log level
// I'm using it for all of main, just just actix-web
fn init_logging(level: u64) -> Result<()> {
    // if RUST_BACKTRACE is set, ignore the arg given and set `trace` no matter what
    let verbosity = if var("RUST_BACKTRACE").unwrap_or("0".into()) == "1" {
        "mifkad=trace"
    } else {
        match level {
            0 => "warn",
            1 => "info",
            2 => "debug",
            3 | _ => "trace",
        }
    };
    if verbosity == "mifkad=trace" {
        set_var("RUST_BACKTRACE", "1");
    };
    set_var("RUST_LOG", verbosity);
    pretty_env_logger::init();
    info!(
        "Set verbosity to {}",
        var("RUST_LOG").chain_err(|| "Failed to set verbosity level")?
    );
    Ok(())
}

fn run() -> Result<()> {
    // TODO - set this with a command-line flag.  For now, info is a good default
    // 0 - warn, 1 - info, 2 - debug, 3+ - trace
    init_logging(1)?;

    // Set up corresponding <DATE>.json file
    let db = init_db()?; // Load this info the AppState

    // actix setup
    let sys = actix::System::new("mifkad");
    let addr = "127.0.0.1:8080";

    HttpServer::new(move || {
        App::new()
            .configure({
                |app| {
                    Cors::for_app(app)
                        .allowed_methods(vec!["GET", "POST"])
                        .allowed_headers(vec![http::header::AUTHORIZATION, http::header::ACCEPT])
                        .allowed_header(http::header::CONTENT_TYPE)
                        .max_age(3600)
                        .resource("/", |r| r.method(http::Method::GET).f(index))
                        // whatever day it is today, or Monday on the weekend
                        .resource("/school/today", |r| r.method(http::Method::GET).f(school_today))
                        // mon||monday, e.g.
                        .resource("/school/{day}", |r| r.method(http::Method::GET).with(school))
                        .register()
                }
            })
            .handler(
                "/mifkad-assets",
                StaticFiles::new("./mifkad-assets/").unwrap(),
            )
            .middleware(middleware::Logger::default())
    }).bind(addr)
        .chain_err(|| "Could not initialize server")?
        .start();
    let _ = sys.run();
    Ok(())
}

fn main() {
    if let Err(ref e) = run() {
        error!("error: {}", e);

        for e in e.iter().skip(1) {
            debug!("caused by: {}", e);
        }

        if let Some(backtrace) = e.backtrace() {
            trace!("backtrace: {:?}", backtrace);
        }

        ::std::process::exit(1);
    }
}

#[cfg(test)]
mod tests {
    #[test]
    fn test_it_compiles() {
        assert!(true)
    }
}
