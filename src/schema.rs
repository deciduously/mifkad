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
    pub name: String,
    pub epected: Expected,
}

use uuid::Uuid;

#[derive(Debug)]
pub enum Expected {
    Core(String), // TODO specific "schedule" type?
    Expected(String),
    Unscheduled,
}

#[derive(Debug)]
pub struct Kid {
    pub id: Uuid,
    pub name: String,
    pub schedule: Vec<Day>,
}

impl Kid {
    pub fn new(name: String) -> Self {
        Self {
            id: Uuid::new_v4(), // random
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
