// data.rs handles reading in the Enrollment: Site report and populating internal data structure

// TODO Xls OR Xlsx
use calamine::{open_workbook, Reader, Xlsx};
use errors::*;
use regex::Regex;
use schema::*;

// enum Row {Class, Kid, Capacity} ?

// pull_rows will read in the Enrollment excel sheet and populate the School
fn scrape_enrollment() -> Result<School> {
    lazy_static! {
        // Define patterns to match
        static ref KID_RE: Regex =
            Regex::new(r"((@|#|&) )?(?P<last>[A-Z]+), (?P<first>[A-Z]+)").unwrap();
        static ref CLASS_RE: Regex = Regex::new(r"CLASSROOM: ([A-Z])").unwrap();
        static ref CAPACITY_RE: Regex = Regex::new(r"CLASS MAXIMUM: (\d+)").unwrap();
    }
    // initialize return struct
    let mut school = School::new();

    // Use calamind to read in the input sheet
    // Decide if this is specified by user via the frontend, or just always dropped into the same location on the filesystem
    let mut excel: Xlsx<_> = open_workbook("sample/sample_enroll_all_detail_week.xlsx").unwrap();

    // Try to get "Sheet1" as `r` - it should always exist
    if let Some(Ok(r)) = excel.worksheet_range("Sheet1") {
        // Process each row
        for row in r.rows() {
            debug!("ROW:");
            use calamine::DataType::*;
            // Column A is either a Class or a Kid
            let column_a = &row[0];
            match column_a {
                String(s) => {
                    // If it's a class, open up a new class
                    // If its a kid, push it to the open class
                    // If it's anything else, ignore it.
                    if CLASS_RE.is_match(&s) {
                        debug!("MATCH CLASS: {}", &s);
                        let caps = CLASS_RE.captures(&s).unwrap();
                        // the capacity is found in Column B
                        let mut capacity: u8;
                        match &row[1] {
                            String(s2) => {
                                let capacity_caps = CAPACITY_RE.captures(&s2).unwrap();
                                capacity = (&capacity_caps[1])
                                    .parse::<u8>()
                                    .chain_err(|| "Unable to parse capacity as u8")?;
                            }
                            _ => {
                                bail!("Column B of Classroom declaration contained unexpected data")
                            }
                        }

                        // create a new Classroom and push it to the school
                        let new_class = Classroom::new(caps[1].to_string(), capacity);
                        debug!("ADDED CLASS: {:?}", &new_class);
                        school.classrooms.push(new_class);
                    } else if KID_RE.is_match(&s) {
                        debug!("MATCH KID: {}", &s);
                        let caps = KID_RE.captures(&s).unwrap();

                        // Reformat name from LAST, FIRST to FIRST LAST
                        let mut name = ::std::string::String::from(&caps["first"]);
                        name.push_str(" ");
                        name.push_str(&caps["last"]);

                        // init Kid datatype
                        let mut new_kid = Kid::new(name);

                        // Add each schedule day
                        new_kid.add_day("mon", &format!("{}", &row[6]));
                        new_kid.add_day("tue", &format!("{}", &row[7]));
                        new_kid.add_day("wed", &format!("{}", &row[8]));
                        new_kid.add_day("thu", &format!("{}", &row[9]));
                        new_kid.add_day("fri", &format!("{}", &row[10]));

                        // push the kid to the latest open class
                        let mut classroom = school.classrooms.pop().expect(
                            "Kid found before classroom declaration - input file malformed",
                        );
                        classroom.push_kid(new_kid);
                        school.classrooms.push(classroom);
                    }
                }
                _ => continue,
            }
        }
    }
    println!("SCHOOL: {:?}", school);
    Ok(school)
}

// Kid(name, mon, tue, wed, thu, fri)
// Room

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn test_open_excel() {
        scrape_enrollment().unwrap();
        assert_eq!("write", "me")
    }
}
