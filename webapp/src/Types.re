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
  kids: ref(array(kid)),
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
            ref(Array.of_list(
              List.filter(
                k => k.schedule.expected == "Extended",
                Array.to_list(r.kids^),
              ),
            )),
        },
        school.classrooms,
      ),
  };

let add_extended_room = (school, classroom) => {
  /* This is our folding fn from get_extended_rooms below.
     It should take a room and a school and either add the new room
     or if a room already exists with the same letter, just add those kids */
  let target = ref(school.classrooms);
  
  if (Array.length(target^) == 0) {
    target := Array.append(target^, Array.make(1, classroom))
  } else {
    let already_included = Array.map(oldr => oldr.letter, school.classrooms);
    let found = ref(false);
    let idx = ref(0)/* This will only be read later if found is toggled to true*/;
    Array.iteri((i, l) =>
                if (classroom.letter == l) {
                  found := true/* use iteri!!!!*/;
                  idx := i;
                },
                already_included);
    if (found^) {
      /* We've already seen this letter - mash the new kid list into the matching existing kid list */
      let old_classroom = Array.get(school.classrooms, idx^);
      let new_classroom = {...classroom, kids: ref(Array.append(old_classroom.kids^, classroom.kids^))};
      Array.set(target^, idx^, new_classroom);
    } else {
      /* This is a new extended day room - add it as-is */
      target := Array.append(target^, Array.make(1, classroom));
    };
  };
    
  {...school, classrooms: target^ };
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
        ref(Array.map(
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
    room.kids^,
  )),
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
      Array.fold_left((acc, k) => acc ++ kid(k), "", classroom.kids^);
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
