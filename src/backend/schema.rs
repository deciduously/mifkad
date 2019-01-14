use regex::Regex;
use std::str::FromStr;

#[derive(Clone, Debug, Deserialize, Serialize)]
pub struct Classroom {
    pub id: u32,
    pub letter: String,
    pub capacity: u8,
    pub collected: bool,
    pub kids: Vec<Kid>,
}

impl Classroom {
    pub fn new(id: u32, letter: String, capacity: u8) -> Self {
        Self {
            id,
            letter,
            capacity,
            collected: false,
            kids: Vec::new(),
        }
    }

    pub fn push_kid(&mut self, kid: Kid) {
        self.kids.push(kid);
    }
}

#[derive(Clone, Debug, Deserialize, Serialize)]
pub struct Day {
    pub expected: Expected,
    pub actual: bool,
}

impl Day {
    pub fn new(schedule: &str) -> Self {
        Self {
            expected: Expected::from_str(schedule).unwrap(),
            actual: true,
        }
    }
}

#[derive(Clone, Debug, Deserialize, PartialEq, Serialize)]
pub enum Expected {
    Core,
    Extended,
    Unscheduled,
    Added,
}

impl FromStr for Expected {
    type Err = ::std::num::ParseIntError;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        lazy_static! {
            static ref PARTIAL_HOUR_RE: Regex = Regex::new(r"(\d+):\d+").unwrap();
        }
        use self::Expected::*;
        // if empty string, that's all we need to know
        if s == "" {
            return Ok(Unscheduled);
        };

        let times: Vec<&str> = s.split(" - ").collect();

        // I don't care about the beginning time, just the end
        // if it's a time like 2:30, take the hour and add one
        let end_str = times[1];
        let end = if PARTIAL_HOUR_RE.is_match(end_str) {
            let part_caps = PARTIAL_HOUR_RE.captures(end_str).unwrap();
            part_caps[1].parse::<u8>()? + 1
        } else {
            times[1].parse::<u8>()?
        };

        if end > 4 {
            Ok(Extended)
        } else {
            Ok(Core)
        }
    }
}

#[derive(Clone, Debug, Deserialize, Serialize)]
pub struct Kid {
    pub id: u32,
    pub name: String,
    pub schedule: Day,
}

impl Kid {
    pub fn new(id: u32, name: String, sched_str: &str) -> Self {
        Self {
            id,
            name,
            schedule: Day::new(sched_str),
        }
    }
}

#[derive(Clone, Debug, Deserialize, Serialize)]
pub struct School {
    pub weekday: Weekday,
    pub classrooms: Vec<Classroom>,
    pub extended_day_config: ExtendedDayConfig,
}

impl School {
    pub fn new(day: Weekday, extended_day_config: ExtendedDayConfig) -> Self {
        Self {
            weekday: day,
            classrooms: Vec::new(),
            extended_day_config,
        }
    }

    // Flip the kid with the given ID's attendance
    pub fn toggle_kid(&mut self, id: u32) {
        for c in &mut self.classrooms {
            for k in &mut c.kids {
                if k.id == id {
                    k.schedule.actual = !k.schedule.actual;
                }
            }
        }
    }

    // Add the given kid to Extended Day
    pub fn addext_kid(&mut self, id: u32) {
        use self::Expected::*;
        for c in &mut self.classrooms {
            for k in &mut c.kids {
                if k.id == id {
                    k.schedule.expected = if k.schedule.expected == Core {
                        Added
                    } else {
                        Core
                    };
                }
            }
        }
    }

    // Toggle the given room's collected status
    pub fn collect_room(&mut self, id: u32) {
        for c in &mut self.classrooms {
            if c.id == id {
                c.collected = !c.collected;
            }
        }
    }
}

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct ExtendedDayConfig {
    entries: Vec<ExtendedDayEntry>,
}

impl Default for ExtendedDayConfig {
    fn default() -> Self {
        Self {
            entries: vec![
                ExtendedDayEntry::new("AE", "7", vec!["A", "C"]),
                ExtendedDayEntry::new("DE", "9", vec!["B", "D"]),
                ExtendedDayEntry::new("EE", "9", vec!["E", "F", "G"]),
                ExtendedDayEntry::new("IE", "14", vec!["J", "K", "H", "I"]),
                ExtendedDayEntry::new("LE", "20", vec!["L", "M", "N", "O"]),
            ],
        }
    }
}

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct ExtendedDayEntry {
    letter: String,
    capacity: String,
    members: Vec<String>,
}

impl ExtendedDayEntry {
    fn new(letter: &str, capacity: &str, members: Vec<&str>) -> Self {
        Self {
            letter: letter.into(),
            capacity: capacity.into(),
            members: members.iter().map(|s| s.to_string()).collect(),
        }
    }
}

#[derive(Clone, Copy, Debug, Deserialize, Serialize)]
pub enum Weekday {
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
}

impl FromStr for Weekday {
    type Err = ::std::io::Error;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        use self::Weekday::*;
        let ret = match s {
            "mon" | "monday" | "Mon" => Monday,
            "tue" | "tuesday" | "Tue" => Tuesday,
            "wed" | "wednesday" | "Wed" => Wednesday,
            "thu" | "thursday" | "Thu" => Thursday,
            "fri" | "friday" | "Fri" => Friday,
            _ => Monday, // if anything else, like a weekend, just run it for Monday - keep the "pick a different day" button
        };
        Ok(ret)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_expected_core() {
        assert_eq!(Expected::from_str("8 - 4").unwrap(), Expected::Core)
    }
    #[test]
    fn test_expected_extended() {
        assert_eq!(Expected::from_str("8 - 6").unwrap(), Expected::Extended)
    }
    #[test]
    fn test_expected_within_core() {
        assert_eq!(Expected::from_str("9 - 1").unwrap(), Expected::Core)
    }
    #[test]
    fn test_expected_core_late_start() {
        assert_eq!(Expected::from_str("10 - 4").unwrap(), Expected::Core)
    }
    #[test]
    fn test_expected_core_partial_endtime() {
        assert_eq!(Expected::from_str("8 - 2:30").unwrap(), Expected::Core)
    }
    #[test]
    fn test_expected_core_early_start() {
        assert_eq!(Expected::from_str("7:30 - 4").unwrap(), Expected::Core)
    }
    #[test]
    fn test_expected_extended_early_start() {
        assert_eq!(Expected::from_str("7:30 - 6").unwrap(), Expected::Extended)
    }
    #[test]
    fn test_expected_extended_late_start() {
        assert_eq!(Expected::from_str("10 - 6").unwrap(), Expected::Extended)
    }
    #[test]
    fn test_expected_unscheduled() {
        assert_eq!(Expected::from_str("").unwrap(), Expected::Unscheduled)
    }
    #[test]
    #[should_panic]
    fn test_expected_unrecognized() {
        assert_eq!(Expected::from_str("8 - w").unwrap(), Expected::Core)
    }
}
