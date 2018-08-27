// handlers.rs defines the actix_web handlers
use actix_web::{fs::NamedFile, HttpRequest, Json, Path, Responder, Result};
use data::scrape_enrollment;
use schema;
use std::{path::PathBuf, str::FromStr};
use super::AppState;

pub fn index(_req: &HttpRequest<AppState>) -> Result<NamedFile> {
    let path: PathBuf = PathBuf::from("./mifkad-assets/index.html");
    Ok(NamedFile::open(path)?)
}

pub fn school_today(_req: &HttpRequest<AppState>) -> impl Responder {
    "TODAY"
}

pub fn school(day: Path<String>) -> impl Responder {
    Json(
        scrape_enrollment(
            schema::Weekday::from_str(&day).expect("Unexpected day passed from URL"),
            "current.xls",
        ).unwrap(),
    )
}
