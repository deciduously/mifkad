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

let kid = kid : string =>
  kid.schedule.actual ? "" : to_fmt_name(kid.name) ++ ", ";

let classroom = classroom : string => {
  let kidlist =
    Array.fold_left((acc, k) => acc ++ kid(k), "", classroom.kids^);
  "Room "
  ++ classroom.letter
  ++ ": "  /* Check if empty, and if it's not empty, trim off the trailing comma */
  ++ (
    String.length(kidlist) > 0 ?
      String.sub(kidlist, 0, String.length(kidlist) - 2) : "All here"
  )
  ++ "\r\n";
};

let trim_trailing = s =>
  /* Trims the trailing comma and space that happens when we grab our kid list strings */
  String.sub(s, 0, String.length(s) - 2);

let ext_classroom = classroom : string => {
  let absent_kids =
    Array.fold_left(
      (acc, k) =>
        k.schedule.actual ?
          acc :
          /* If the kid's a pink sheet, no need to say they were going to be added */
          k.schedule.expected == "Added" ? acc : acc ++ kid(k),
      "",
      classroom.kids^,
    );
  let none_absent = String.length(absent_kids) == 0;
  let added_kids =
    Array.fold_left(
      (acc, k) =>
        k.schedule.expected == "Added" ?
          /* If the kid is absent, no need to say they're added */
          k.schedule.actual ? acc ++ to_fmt_name(k.name) ++ ", " : acc : acc,
      "",
      classroom.kids^,
    );
  let none_added = String.length(added_kids) == 0;
  let absent_str = none_absent ? "" : trim_trailing(absent_kids);
  let added_str = none_added ? "" : trim_trailing(added_kids);

  "Room "
  ++ String.sub(classroom.letter, 0, 1 /* Trim the trailing "E" */)
  ++ ": "
  ++ string_of_int(
       /* Expected minus absent */ Array.length(classroom.kids^)
       - List.length(
           List.filter(
             k => ! k.schedule.actual,
             Array.to_list(classroom.kids^),
           ),
         ),
     )
  ++ (none_absent ? " " : " [No: " ++ absent_str)
  ++ (none_added ? "" : (! none_absent ? "; " : "[") ++ "Add: " ++ added_str)
  ++ (none_added && none_absent ? "" : "]")
  ++ "\r\n";
};

let school = (school, extended_config) : string => {
  let date = [%raw
    {|
      function() {
        var d = new Date();
        var s = d.toLocaleDateString();
        return s;
      }
    |}
  ];
  let ext = get_extended_rooms(school, extended_config);
  let uncollected_rooms = get_uncollected_rooms(school);
  let uncollected_str =
    String.length(uncollected_rooms) >= 1 ?
      "Double check the following rooms:\r\n" ++ uncollected_rooms ++ "\r\n" :
      "";
  uncollected_str
  ++ Array.fold_left(
       (acc, room) => acc ++ classroom(room),
       "",
       school.classrooms,
     )
  ++ "\r\nHi Everyone,\r\nHere are your extended day numbers for "
  ++ date()
  ++ ":\r\n\r\n"
  ++ Array.fold_left(
       (acc, room) => acc ++ ext_classroom(room),
       "",
       ext.classrooms,
     )
  ++ "\r\nThanks,\r\n";
};
