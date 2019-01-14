use errors::*;
use std::{fmt, path::PathBuf};
use toml;
use util::*;

static DEFAULT_CONFIG: &str = "Mifkad.toml";

#[derive(Clone, Copy, Debug, Deserialize, PartialEq, Serialize)]
pub enum Verbosity {
    Warn,
    Info,
    Debug,
    Trace,
}

#[derive(Clone, Debug, Deserialize, PartialEq, Serialize)]
pub struct Config {
    pub port: u16,
    pub roster: PathBuf,
    pub extended_config: PathBuf,
    pub verbosity: Verbosity,
}

impl fmt::Display for Config {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(
            f,
            "Roster: {} - Extended Day Setup: {} - Verbosity: {:?} - Port: {}",
            self.roster.to_str().unwrap(),
            self.extended_config.to_str().unwrap(),
            self.verbosity,
            self.port
        )
    }
}

impl Default for Config {
    fn default() -> Self {
        Self {
            port: 8080,
            roster: "current.xls".into(),
            extended_config: "extended_day.json".into(),
            verbosity: Verbosity::Warn,
        }
    }
}

pub fn init_config(s: Option<&str>) -> Result<Config> {
    Ok(
        toml::from_str(&file_contents_from_str_path(s.unwrap_or(DEFAULT_CONFIG))?)
            .chain_err(|| "Improperly formatted TOML")?,
    )
}
