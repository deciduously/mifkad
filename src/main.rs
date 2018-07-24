extern crate actix;
extern crate actix_web;
#[macro_use]
extern crate error_chain;
#[macro_use]
extern crate log;
#[cfg(test)]
extern crate pretty_assertions;
extern crate pretty_env_logger;

mod errors {
    error_chain!{}
}

use actix_web::{
    fs::NamedFile, http, middleware::{self, cors::Cors}, server::HttpServer, App, HttpRequest,
};
use errors::*;
use std::{
    env::{set_var, var}, path::PathBuf,
};

fn init_logging(level: u64) -> Result<()> {
    let verbosity = match level {
        0 => "warn",
        1 => "info",
        2 => "debug",
        3 | _ => "trace",
    };
    if verbosity == "trace" {
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
    // Start env_logger - for now, change this number to change log level
    // I'm using it for all of main, just just actix-web
    init_logging(1)?;

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
                        .resource("/", |r| r.method(http::Method::GET).with(index))
                        .register()
                }
            })
            .handler(
                "/",
                actix_web::fs::StaticFiles::new("./static").index_file("index.html"),
            )
            .middleware(middleware::Logger::default())
    }).bind(addr)
        .chain_err(|| "Could not initialize server")?
        .start();
    let _ = sys.run();
    Ok(())
}

fn index(_req: HttpRequest) -> actix_web::Result<NamedFile> {
    let path: PathBuf = PathBuf::from("./webapp/src/index.html");
    Ok(NamedFile::open(path)?)
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
