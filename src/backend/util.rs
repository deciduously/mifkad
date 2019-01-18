use crate::errors::*;
use std::{
    fs::File,
    io::{BufReader, Read},
};

pub fn file_contents_from_str_path(s: &str) -> Result<String> {
    use std::{fs::File, path::Path};

    let f = File::open(Path::new(&s)).chain_err(|| format!("Could not open {}", s))?;
    string_of_file(&f)
}

pub fn string_of_file(f: &File) -> Result<String> {
    let mut bfr = BufReader::new(f);
    let mut ret = String::new();
    let _ = bfr.read_to_string(&mut ret);
    Ok(ret)
}
