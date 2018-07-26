// enrollment.rs handles reading in the Enrollment: Site report and populating internal data structure

// TODO Xls OR Xlsx
use calamine::{open_workbook, Reader, Xlsx};
use errors::*;

#[derive(Debug)]
pub enum ExpectedEnrollment {
    Core,
    Extended,
    None,
}

pub fn read_excel() -> Result<()> {
    let mut excel: Xlsx<_> = open_workbook("sample/sample_enroll_all_detail_week.xlsx").unwrap();
    if let Some(Ok(r)) = excel.worksheet_range("Sheet1") {
        let mut idxs = String::new();
        for row in r.rows() {
            //println!("row={:?}, row[0]={:?}", row, row[0]);
            idxs.push_str(&format!("{:?}\n", row[0]))
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
        read_excel();
        assert_eq!("write", "me")
    }
}
