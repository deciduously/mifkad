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
    }
    // initialize return struct
    let mut school = School::new();

    // Use calamind to read in the input sheet
    // Decide if this is specified by user via the frontend, or just always dropped into the same location on the filesystem
    let mut excel: Xlsx<_> = open_workbook("sample/sample_enroll_all_detail_week.xlsx").unwrap();

    // Try to get "Sheet1" as `r` - it should always exist
    if let Some(Ok(r)) = excel.worksheet_range("Sheet1") {
        // This is just for debugginging - shoudl be removed
        let mut idxs = String::new();

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
                        idxs.push_str(&format!("ROOM {}:\n", &caps[1]));

                        // create a new Classroom and push it to the school
                        let new_class = Classroom::new(caps[1].to_string(), 2); // TODO capacity -  I think its in a different column
                        debug!("ADDED CLASS: {:?}", &new_class);
                        school.classrooms.push(new_class);
                    } else if KID_RE.is_match(&s) {
                        debug!("MATCH KID: {}", &s);
                        let caps = KID_RE.captures(&s).unwrap();
                        idxs.push_str(&format!("{} {}\n", &caps["first"], &caps["last"]));
                        let mut name = ::std::string::String::from(&caps["first"]);
                        name.push_str(" ");
                        name.push_str(&caps["last"]);

                        println!("NAME: {}", name);
                        let new_kid = Kid::new(name);

                        // push the kid to the latest open class
                        let mut classroom =
                            school.classrooms.pop().expect("No open classroom to pop");
                        classroom.push_kid(new_kid);
                        school.classrooms.push(classroom);
                    }
                }
                _ => continue,
            }
            //println!("row={:?}, row[0]={:?}", row, row[0]);
        }
        println!("{}", idxs)
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
