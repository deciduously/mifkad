use errors::*;
use std::{fmt, path::PathBuf, str::FromStr};
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

impl FromStr for Verbosity {
    type Err = ::std::io::Error;
    fn from_str(s: &str) -> ::std::result::Result<Self, Self::Err> {
        match s {
            "info" | "Info" | "1" => Ok(Verbosity::Info),
            "debug" | "Debug" | "2" => Ok(Verbosity::Debug),
            "trace" | "Trace" | "3" => Ok(Verbosity::Trace),
            "warn" | "Warn" | "0" | _ => Ok(Verbosity::Warn),
        }
    }
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
            "Port {} :: Roster {:?} :: Extended Day Setup {:?} :: Verbosity {:?}",
            self.port, self.roster, self.extended_config, self.verbosity
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

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_init_config_default() {
        assert_eq!(init_config(None).unwrap(), Config::default())
    }
}
