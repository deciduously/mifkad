/* Types.re holds types and transformations used throughout the UI components */

/* payload types to match types in /src/schema.rs */

/* type expected = Core | Extended | Unscheduled; */

/* Type declarations */
type day = {
  weekday: string,
  expected: string,
  actual: bool,
};

type kid = {
  name: string,
  schedule: day,
};

type classroom = {
  letter: string,
  capacity: int,
  kids: array(kid),
};

type school = {
  classrooms: array(classroom),
  weekday: string,
};

let get_extended_letter = letter =>
  switch (letter) {
  | "A" => "AE"
  | "C" => "CE"
  | "B"
  | "D" => "DE"
  | "E"
  | "F"
  | "G" => "EE"
  | "I"
  | "J"
  | "K" => "JE"
  | "N"
  | "O" => "NE"
  | _ => "ERR!!!"
  };

let get_extended_kids = school =>
  /* Returns a school with only the Extended Day kids */
  {
    ...school,
    classrooms:
      Array.map(
        r => {
          ...r,
          letter: get_extended_letter(r.letter),
          kids:
            Array.of_list(
              List.filter(
                k => k.schedule.expected == "Extended",
                Array.to_list(r.kids),
              ),
            ),
        },
        school.classrooms,
      ),
  };

let add_extended_room = (school, classroom) => {
  let added = false;
  let ret = {
    ...school,
    classrooms:
      Array.fold_left(
    (rooms, oldr) =>
      if (Array.length(rooms) == 0) {
         /* The first time through, just add the first room as-is */ Array.append(rooms, Array.make(1, oldr))
       } else {
         /* Otherwise, check if we're a duplicate and then either pop it in or add the new kids to the existing entry */
         let last_room = Array.get(rooms, Array.length(rooms) - 1)/* BEN you dont want to look at the last one, you want to look at ALL of them and see iftheres a match */;
         oldr.letter == last_room.letter
          ? /* BEN you don't want to append here, jut mutate the existing
           but that makes your branch types mismatch.  I think you set a flag and deal with it after the fold - do nothing inside here
           also, toggle Added - that might be a large part of your problem */
           Array.append(rooms, Array.make(1, {...last_room, kids: Array.append(last_room.kids, Array.append(oldr.kids, last_room.kids))}))
          : Array.append(rooms, Array.make(1, oldr));
       },
    [||],
    school.classrooms,
  )};
  if (! added)/* Heres part of the problem - you never flip this marker*/ {
    {
    /* append to end as new class */
    ...ret,
    classrooms: Array.append(school.classrooms, Array.make(1, classroom)),
  };
  } else {
    ret;
  };
};

let get_extended_rooms = school => {
  /* Returns a `school` of the extended kids */
  let s = get_extended_kids(school);
  Array.fold_left(
    add_extended_room,
    {...school, classrooms: [||]},
    s.classrooms,
  );
};

let toggle = (school, kid) => {
  /* Returns a new school with the specified kid toggled */
  ...school,
  classrooms:
    Array.map(
  room => {
      ...room,
      kids:
        Array.map(
    k =>
      if (kid == k) {
        {
        ...kid,
        schedule: {
          ...kid.schedule,
          actual: ! kid.schedule.actual,
        },
      };
      } else {
        k;
      },
    room.kids,
  ),
    },
  school.classrooms,
),
};

module Report = {
  /* Written to match output from the original ClojureScript version */
  /* Returns a single string reporting the school's attendance */
  /* original CLJS found at https://github.com/deciduously/attendance/blob/master/src/cljs/attendance/report.cljs */

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
      Array.fold_left((acc, k) => acc ++ kid(k), "", classroom.kids);
    "Room "
    ++ classroom.letter
    ++ ": "  /* Check if empty, and if it's not empty, trim off the trialing comma */
    ++ (
      String.length(kidlist) > 0 ?
        String.sub(kidlist, 0, String.length(kidlist) - 2) : "All here"
    )
    ++ "\n";
  };

  let school = school : string =>
    Array.fold_left(
      (acc, room) => acc ++ classroom(room),
      "",
      school.classrooms,
    );
};
