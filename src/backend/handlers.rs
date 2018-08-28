// handlers.rs defines the actix_web handlers
//use super::AppState;
use super::AppState;
use actix_web::{self, fs::NamedFile, HttpRequest, HttpResponse, Json, Path};
use data::scrape_enrollment;
use futures::future::{result, Future};
use schema;
use std::{
    clone::Clone,
    io::{prelude::*, BufReader},
    path::PathBuf,
    str::FromStr,
};

pub fn index(
    _req: &HttpRequest<AppState>,
) -> Box<Future<Item = HttpResponse, Error = actix_web::Error>> {
    let path: PathBuf = PathBuf::from("./mifkad-assets/index.html");

    let f = NamedFile::open(&path).expect(&format!("Could not open {}", path.to_str().unwrap()));
    let mut bfr = BufReader::new(f.file());
    let mut ret = String::new();
    bfr.read_to_string(&mut ret)
        .expect(&format!("could not read index file"));

    Box::new(result(Ok(HttpResponse::Ok()
        .content_type("text/html")
        .body(ret))))
}

// The default handler used by the app
pub fn school_today(
    req: &HttpRequest<AppState>,
) -> Box<Future<Item = Json<schema::School>, Error = actix_web::Error>> {
    let a = &req.state().school;
    let school = a.read().unwrap();
    let ret = Json((*school).clone());
    Box::new(result(Ok(ret)))
}

// This was used if the user specifically asks to pick a different day
// Leaving here in case they do actually want that option.
// NOTE - this doesn't affect the AppState, so it wont work with persistence.
// You need to have this replace the app state
pub fn school(
    day: Path<String>,
) -> Box<Future<Item = Json<schema::School>, Error = actix_web::Error>> {
    Box::new(result(Ok(Json(
        scrape_enrollment(
            schema::Weekday::from_str(&day).expect("Unexpected day passed from URL"),
            "current.xls",
        ).unwrap(),
    ))))
}
