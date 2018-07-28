use regex::Regex;
use std::str::FromStr;

#[derive(Debug, Serialize)]
pub struct Classroom {
    pub letter: String,
    pub capacity: u8,
    pub kids: Vec<Kid>,
}

impl Classroom {
    pub fn new(letter: String, capacity: u8) -> Self {
        Self {
            letter,
            capacity,
            kids: Vec::new(),
        }
    }

    pub fn push_kid(&mut self, kid: Kid) {
        self.kids.push(kid);
    }
}

#[derive(Debug, Serialize)]
pub struct Day {
    pub weekday: Weekday,
    pub expected: Expected,
}

impl Day {
    pub fn new(day: &str, schedule: &str) -> Self {
        Self {
            weekday: Weekday::from_str(day).unwrap(),
            expected: Expected::from_str(schedule).unwrap(),
        }
    }
}

// TODO carry the actual schedule with this
// Unimportant for now - we dont care beyond whether or not they go to extended
#[derive(Debug, PartialEq, Serialize)]
pub enum Expected {
    Core,
    Extended,
    Unscheduled,
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

#[derive(Debug, Serialize)]
pub struct Kid {
    //pub id: Uuid,
    pub name: String,
    pub schedule: Vec<Day>,
}

impl Kid {
    pub fn new(name: String) -> Self {
        Self {
            //id: Uuid::new_v4(), // random
            name,
            schedule: Vec::new(),
        }
    }
    pub fn add_day(&mut self, day_str: &str, sched_str: &str) {
        // TODO check if day already exists - therese should be unique
        // a HashMap is probably a better idea, luckily we can keep this fn
        self.schedule.push(Day::new(day_str, sched_str));
    }
}

#[derive(Debug, Serialize)]
pub struct School {
    pub classrooms: Vec<Classroom>,
}

impl School {
    pub fn new() -> Self {
        Self {
            classrooms: Vec::new(),
        }
    }
}

#[derive(Debug, Serialize)]
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
            "mon" | "monday" => Monday,
            "tue" | "tuesday" => Tuesday,
            "wed" | "wednesday" => Wednesday,
            "thu" | "thursday" => Thursday,
            "fri" | "friday" => Friday,
            _ => {
                return Err(::std::io::Error::new(
                    ::std::io::ErrorKind::Other,
                    "Unknown weekday",
                ));
            }
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
