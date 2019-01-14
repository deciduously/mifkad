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
extern crate toml;

mod config;
mod data;
mod errors {
    error_chain! {}
}
mod handlers;
mod schema;
mod util;

use actix_web::{
    fs::StaticFiles,
    http,
    middleware::{self, cors::Cors},
    server::HttpServer,
    App,
};
use config::{init_config, Config};
use data::init_db;
use errors::{Result, ResultExt};
use handlers::{adjust_school, index, school_today};
use schema::School;
use std::{
    env::{set_var, var},
    sync::{Arc, RwLock},
};

// RwLock allows either multiple readers or a single writer, but not both
pub struct AppState {
    pub config: Config,
    pub school: Arc<RwLock<School>>,
}

impl AppState {
    fn new(a: &Arc<RwLock<School>>, config: &Config) -> Result<Self> {
        let school = Arc::clone(a);
        Ok(Self {
            config: config.clone(),
            school,
        })
    }
}

// Start env_logger
// I'm using it for all of main, not just actix-web
fn init_logging(level: config::Verbosity) -> Result<()> {
    // if RUST_BACKTRACE is set, ignore the arg given and set `trace` no matter what
    let verbosity = if var("RUST_BACKTRACE").unwrap_or_else(|_| "0".into()) == "1" {
        "mifkad=trace"
    } else {
        use config::Verbosity::*;
        match level {
            Warn => "warn",
            Info => "info",
            Debug => "debug",
            Trace => "trace",
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
    let config = init_config(None).unwrap_or_default();
    info!("{}", config);
    init_logging(config.verbosity)?;

    let initial_school = Arc::new(RwLock::new(init_db(&config)?));

    // actix setup
    let sys = actix::System::new("mifkad");
    let addr = format!("127.0.0.1:{}", config.port);

    HttpServer::new(move || {
        App::with_state(
            AppState::new(&initial_school, &config).expect("could not initialize AppState"),
        )
        .configure({
            |app| {
                Cors::for_app(app)
                    .send_wildcard()
                    .allowed_methods(vec!["GET"])
                    .max_age(3600)
                    .resource("/", |r| r.route().a(index))
                    .resource("/school/today", |r| {
                        r.method(http::Method::GET).a(school_today)
                    })
                    .resource("/{action}/{id}", |r| {
                        r.method(http::Method::GET).with(adjust_school)
                    })
                    .register()
            }
        })
        .handler(
            "/mifkad-assets",
            StaticFiles::new("./mifkad-assets/").unwrap(),
        )
        .middleware(middleware::Logger::default())
    })
    .bind(addr)
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
