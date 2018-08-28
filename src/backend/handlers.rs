// handlers.rs defines the actix_web handlers
//use super::AppState;
use super::AppState;
use actix_web::{
    self, fs::NamedFile, AsyncResponder, HttpMessage, HttpRequest, HttpResponse, Json, Path,
};
use data::scrape_enrollment;
use futures::future::{result, Future};
use schema;
use std::{
    clone::Clone,
    io::{prelude::Read, BufReader},
    path::PathBuf,
    str::FromStr,
};

// The types of actions adjust_school knows how to do
#[derive(Debug, Deserialize, Serialize)]
enum Action {
    // Flip a kid's attendance
    Toggle(u32),
    // Flip a kid's expected from "Core" to "Actual" or vice versa
    AddExt(u32),
    // Flip a room's collected field
    Collect(u32),
}

pub fn index(
    _req: &HttpRequest<AppState>,
) -> Box<Future<Item = HttpResponse, Error = actix_web::Error>> {
    let path: PathBuf = PathBuf::from("./mifkad-assets/index.html");

    let f = NamedFile::open(&path)
        .unwrap_or_else(|_| panic!("Could not open {}", path.to_str().unwrap()));
    let mut bfr = BufReader::new(f.file());
    let mut ret = String::new();
    bfr.read_to_string(&mut ret)
        .unwrap_or_else(|_| panic!("could not read index file"));

    result(Ok(HttpResponse::Ok().content_type("text/html").body(ret))).responder()
}

// The RwLock read handler
pub fn school_today(
    req: &HttpRequest<AppState>,
) -> Box<Future<Item = Json<schema::School>, Error = actix_web::Error>> {
    let a = &req.state().school.read().unwrap();
    let ret = Json((*a).clone());
    result(Ok(ret)).responder()
}

// the RwLock write handler
pub fn adjust_school(
    req: &HttpRequest<AppState>,
) -> Box<Future<Item = HttpResponse, Error = actix_web::Error>> {
    req.json()
        .from_err()
        .and_then(|val: Action| {
            println!("{:?}", val);
            let ret = schema::School::new(schema::Weekday::Monday);
            Ok(HttpResponse::Ok().json(ret))
        })
        .responder()
}

// This was used if the user specifically asks to pick a different day
// Leaving here in case they do actually want that option.
// NOTE - this doesn't affect the AppState, so it wont work with persistence.
// You need to have this replace the app state
pub fn school(
    day: Path<String>,
) -> Box<Future<Item = Json<schema::School>, Error = actix_web::Error>> {
    result(Ok(Json(
        scrape_enrollment(
            schema::Weekday::from_str(&day).expect("Unexpected day passed from URL"),
            "current.xls",
        ).unwrap(),
    ))).responder()
}
