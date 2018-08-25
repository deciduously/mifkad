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
use chrono::prelude::*;
use errors::*;
use handlers::*;
use std::{
    cell::Cell,
    env::{set_var, var},
    fs::{create_dir, read_dir, File},
    io::{prelude::*, BufReader},
    path::{Path, PathBuf},
};

static DATAFILE: &str = "current.xls";

struct AppState {
    school: Cell<schema::School>,
}

// Determine what day it is, and either write a new db file or read the one there
// It returns the school to load in to the AppState
fn init_db() -> Result<(schema::School)> {
    // First, grab today's date and the day of the week
    let today = Local::today();
    // I don't just use chrono::Weekday because my custom type schema::Weekday only likes Mon-Fri, anything else will default to Mon
    let day_of_week = &format!("{:?}", today.weekday());
    let day_str = format!("{}{}{}", today.year(), today.month(), today.day());
    let filename = format!("{}.json", day_str);

    // Then, open up our db folder in mifkad-assets.  If it doesnt exist, create it
    // First, we need to build the path - I'm going to hardcode this, it's unlikely to move
    let mut db_path = PathBuf::new();
    db_path.push("mifkad-assets");
    db_path.push("db");
    let mut filepath = db_path.clone();
    filepath.push(&filename);

    if !&db_path.exists() {
        warn!("No db found!  Creating...");
        create_dir(&db_path).chain_err(|| "Could not create mifkad-assets\db")?;
    }

    // Now, check if we have an entry for today.  If it doesn't exist, write it from the GAN data
    // First, get the contents of the directory
    let dir_listing: Vec<PathBuf> = read_dir(&db_path)
        .chain_err(|| "could not read db!")?
        .map(|f| f.expect("could not read db entry").path())
        .collect();

    // Try to locate today.
    let mut found = false;
    for l in &dir_listing {
        let curr_str = l.to_str().unwrap();
        if curr_str == filepath.to_str().unwrap() {
            info!(
                "Found previously logged attendance for {}, loading...",
                &day_str
            );
            found = true;
            break;
        }
    }

    // If we didn't find a corresponding file, serialize it out from DATAFILE
    if !found {
        info!("No record found for {}.  Reading {}", day_str, DATAFILE);
        let school = json!(data::scrape_enrollment(day_of_week, DATAFILE)?);
        let mut new_f = File::create(filepath.to_str().unwrap())
            .chain_err(|| format!("could not create {}", &filename))?;
        new_f
            .write_all(school.to_string().as_bytes())
            .chain_err(|| format!("could not write data to {}", &filename))?;
    }

    // Finally, read in today's entry from the database, which we've ensured exists
    let f = File::open(Path::new(&filepath)).chain_err(|| format!("could not open {}", &filename))?;
    let mut bfr = BufReader::new(f);
    let mut input_str = String::new();
    let _ = bfr.read_to_string(&mut input_str);

    // Deserialize the file contents and return
    let ret: schema::School = serde_json::from_str(&input_str)
        .chain_err(|| format!("could not parse contents of {}", &filename))?;

    info!(
        "Mifkad initialized - using file {}",
        filepath.to_str().unwrap()
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
    let _sc = init_db()?; // Load this info the AppState

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
