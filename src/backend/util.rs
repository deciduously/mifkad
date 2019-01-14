use errors::*;
use std::io::{BufReader, Read};

pub fn file_contents_from_str_path(s: &str) -> Result<String> {
    use std::{fs::File, path::Path};

    let f = File::open(Path::new(&s)).chain_err(|| format!("Could not open {}", s))?;
    let mut bfr = BufReader::new(f);
    let mut input = String::new();
    let _ = bfr.read_to_string(&mut input);

    Ok(input)
}
