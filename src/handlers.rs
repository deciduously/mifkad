// handlers.rs defines the actix_web handlers
use actix_web::{fs::NamedFile, HttpRequest, Result};
use std::path::PathBuf;

pub fn index(_req: HttpRequest) -> Result<NamedFile> {
    let path: PathBuf = PathBuf::from("./webapp/src/index.html");
    Ok(NamedFile::open(path)?)
}
