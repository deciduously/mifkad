use std::str::FromStr;

#[derive(Debug)]
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

#[derive(Debug)]
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

#[derive(Debug, PartialEq)]
pub enum Expected {
    Core(String), // TODO specific "schedule" type?
    Extended(String),
    Unscheduled,
}

impl FromStr for Expected {
    type Err = ::std::num::ParseIntError;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        use self::Expected::*;
        // if empty string, that's all we need to know
        if s == "" {return Ok(Unscheduled)};

        let times: Vec<&str> = s.split('-').collect();
        // I don't care about the begining time
        // If this tool ever needs to care about Early Drop, you'll need to deal with 7:30 - otherwise all numbers are whole
        //let _begin = times[0].parse::<u8>()?;
        let end = times[1].parse::<u8>()?;
        if end > 4 {
            Ok(Extended(s.to_string()))
        } else {
            Ok(Core(s.to_string()))
        }
    }
}

#[derive(Debug)]
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
}

#[derive(Debug)]
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

#[derive(Debug)]
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
            _ => { return Err(::std::io::Error::new(::std::io::ErrorKind::Other, "Unknown weekday")); },
        };
    Ok(ret)
    }    
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_expected_core() {
        assert_eq!(Expected::from_str("8-4").unwrap(), Expected::Core("8-4".to_string()))
    }
    #[test]
    fn test_expected_extended() {
        assert_eq!(Expected::from_str("8-6").unwrap(), Expected::Extended("8-6".to_string()))
    }
    #[test]
    fn test_expected_within_core() {
        assert_eq!(Expected::from_str("9-1").unwrap(), Expected::Core("9-1".to_string()))
    }
    #[test]
    fn test_expected_core_late_start() {
        assert_eq!(Expected::from_str("10-4").unwrap(), Expected::Core("10-4".to_string()))
    }
    #[test]
    fn test_expected_core_early_start() {
        assert_eq!(Expected::from_str("7:30-4").unwrap(), Expected::Core("7:30-4".to_string()))
    }
    #[test]
    fn test_expected_extended_early_start() {
        assert_eq!(Expected::from_str("7:30-6").unwrap(), Expected::Extended("7:30-6".to_string()))
    }
    #[test]
    fn test_expected_extended_late_start() {
        assert_eq!(Expected::from_str("10-6").unwrap(), Expected::Extended("10-6".to_string()))
    }
    #[test]
    fn test_expected_unscheduled() {
        assert_eq!(Expected::from_str("").unwrap(), Expected::Unscheduled)
    }
    #[test]
    #[should_panic]
    fn test_expected_unrecognized() {
        assert_eq!(Expected::from_str("8-w").unwrap(), Expected::Core("8-w".to_string()))
    }
}
