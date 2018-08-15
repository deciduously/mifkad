/* Written to match output from the original ClojureScript version */
/* Returns a single string reporting the school's attendance */
/* original CLJS found at https://github.com/deciduously/attendance/blob/master/src/cljs/attendance/report.cljs */

open Types;

let to_fmt_name = name => {
  /* Takes FIRSTNAME LASTNAME and returns Firstname L. */
  let idx_of_spc = String.index(name, ' ');

  let first_name =
    String.sub(name, 0, idx_of_spc)  /* start_idx, len */
    |> String.lowercase
    |> String.capitalize;

  let last_initial = String.sub(name, idx_of_spc + 1, 1);

  first_name ++ " " ++ last_initial ++ ".";
};

let kid = kid: string =>
  kid.schedule.actual ? "" : to_fmt_name(kid.name) ++ ", ";

let classroom = classroom: string => {
  let kidlist =
    Array.fold_left((acc, k) => acc ++ kid(k), "", classroom.kids^);
  "Room "
  ++ classroom.letter
  ++ ": "  /* Check if empty, and if it's not empty, trim off the trialing comma */
  ++ (
    String.length(kidlist) > 0 ?
      String.sub(kidlist, 0, String.length(kidlist) - 2) : "All here"
  )
  ++ "\r\n";
};

let ext_kid = kid: string =>
  kid.schedule.actual ? "" : to_fmt_name(kid.name) ++ ", ";

let ext_classroom = classroom: string => {
  let absent_kids =
    Array.fold_left(
      (acc, k) => k.schedule.actual ? acc : acc ++ kid(k),
      "",
      classroom.kids^,
    );
  let absent_str =
    /* Check if empty, and if not trim off trailing comma and put in brackets */
    String.length(absent_kids) > 0 ?
      "[No: "
      ++ String.sub(absent_kids, 0, String.length(absent_kids) - 2)
      ++ "]" :
      "";
  "Room "
  ++ classroom.letter
  ++ ": "
  ++ string_of_int(
       /* Expected minus absent */ Array.length(classroom.kids^)
       - List.length(
           List.filter(
             k => !k.schedule.actual,
             Array.to_list(classroom.kids^),
           ),
         ),
     )
  ++ " "
  ++ absent_str
  ++ "\r\n";
};

let school = school: string => {
  let ext = get_extended_rooms(school);
  Array.fold_left(
    (acc, room) => acc ++ classroom(room),
    "",
    school.classrooms,
  )
  ++ "\r\nExtended Day:\r\n"
  ++ Array.fold_left(
       (acc, room) => acc ++ ext_classroom(room),
       "",
       ext.classrooms,
     );
};
