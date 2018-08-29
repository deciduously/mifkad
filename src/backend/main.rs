extern crate actix;
extern crate actix_web;
extern crate calamine;
extern crate chrono;
#[macro_use]
extern crate error_chain;
extern crate futures;
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
use data::init_db;
use errors::{Result, ResultExt};
use handlers::{adjust_school, index, school, school_today};
use std::{
    env::{set_var, var},
    sync::{Arc, RwLock},
};

static DATAFILE: &str = "current.xls";

// RwLock allows either multiple readers or a single writer, but not both
pub struct AppState {
    pub school: Arc<RwLock<schema::School>>,
}

impl AppState {
    fn new(a: &Arc<RwLock<schema::School>>) -> Result<Self> {
        let school = Arc::clone(a);
        Ok(Self { school })
    }
}

// Start env_logger - for now, change this number to change log level
// I'm using it for all of main, just just actix-web
fn init_logging(level: u64) -> Result<()> {
    // if RUST_BACKTRACE is set, ignore the arg given and set `trace` no matter what
    let verbosity = if var("RUST_BACKTRACE").unwrap_or_else(|_| "0".into()) == "1" {
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

    let initial_school = Arc::new(RwLock::new(init_db()?));

    // actix setup
    let sys = actix::System::new("mifkad");
    let addr = "127.0.0.1:8080";

    HttpServer::new(move || {
        App::with_state(
            AppState::new(&Arc::clone(&initial_school)).expect("could not initialize AppState"),
        ).configure({
            |app| {
                Cors::for_app(app)
                        .allowed_methods(vec!["GET", "POST"])
                        .allowed_headers(vec![http::header::AUTHORIZATION, http::header::ACCEPT])
                        .allowed_header(http::header::CONTENT_TYPE)
                        .max_age(3600)
                        .resource("/", |r| r.route().a(index)) // a() registers an async handler, which is a Box<Future<Item=impl Responder, actix_web::Error>>
                        .resource("/school/today", |r| r.method(http::Method::GET).a(school_today))
                        // Mon||mon||monday, e.g. - this is vestigial and may be removed
                        .resource("/school/{day}", |r| r.method(http::Method::GET).with(school))
                        .resource("/school/adjust", |r| r.method(http::Method::POST).with(adjust_school))
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
