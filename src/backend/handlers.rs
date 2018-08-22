// handlers.rs defines the actix_web handlers
use actix_web::{fs::NamedFile, HttpRequest, Json, Path, Responder, Result};
use data::scrape_enrollment;
use std::path::PathBuf;

pub fn index(_req: HttpRequest) -> Result<NamedFile> {
    let path: PathBuf = PathBuf::from("./webapp/src/index.html");
    Ok(NamedFile::open(path)?)
}

pub fn school(day: Path<String>) -> impl Responder {
    Json(scrape_enrollment(&day, "current.xls").unwrap())
}
