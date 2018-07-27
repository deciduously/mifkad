// data.rs handles reading in the Enrollment: Site report and populating internal data structure

// TODO Xls OR Xlsx
use calamine::{open_workbook, Reader, Xlsx};
use errors::*;
use regex::Regex;

// temporary
fn print_significant_rows() -> Result<()> {
    lazy_static! {
        static ref KID_RE: Regex =
            Regex::new(r"((@|#|&) )?(?P<last>[A-Z]+), (?P<first>[A-Z]+)").unwrap();
        static ref CLASS_RE: Regex = Regex::new(r"CLASSROOM: ([A-Z])").unwrap();
    }

    let mut excel: Xlsx<_> = open_workbook("sample/sample_enroll_all_detail_week.xlsx").unwrap();
    // Try to get "Sheet1" as `r` - it should always exist
    if let Some(Ok(r)) = excel.worksheet_range("Sheet1") {
        let mut idxs = String::new();
        for row in r.rows() {
            debug!("ROW:");
            use calamine::DataType::*;
            // Column A is either a Class or a Kid
            let column_a = &row[0];
            match column_a {
                String(s) => {
                    // If its a kid, push it to the open class
                    // If it's anything else, ignore it.
                    if KID_RE.is_match(&s) {
                        debug!("MATCH KID: {}", &s);
                        let caps = KID_RE.captures(&s).unwrap();
                        idxs.push_str(&format!("{} {}\n", &caps["first"], &caps["last"]));
                    } else if CLASS_RE.is_match(&s) {
                        debug!("MATCH CLASS: {}", &s);
                        let caps = CLASS_RE.captures(&s).unwrap();
                        idxs.push_str(&format!("ROOM {}:\n", &caps[1]))
                    }
                }
                _ => continue,
            }
            //println!("row={:?}, row[0]={:?}", row, row[0]);
        }
        println!("{}", idxs)
    }
    Ok(())
}

// Kid(name, mon, tue, wed, thu, fri)
// Room

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn test_open_excel() {
        print_significant_rows().unwrap();
        assert_eq!("write", "write")
    }
}
