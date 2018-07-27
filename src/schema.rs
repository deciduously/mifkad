#[derive(Debug)]
pub struct Day {
    pub name: String,
    pub epected: Expected,
}

#[derive(Debug)]
pub enum Expected {
    Core(String), // TODO specific "schedule" type?
    Expected(String),
    Unscheduled,
}

#[derive(Debug)]
pub struct Kid {
    pub id: u32,
    pub name: String,
    pub schedule: Vec<Day>,
}

#[derive(Debug)]
pub struct Classroom {
    pub letter: String,
    pub capacity: u8,
    pub kids: Vec<Kid>,
}

#[derive(Debug)]
pub struct School {
    pub classrooms: Vec<Classroom>,
}
