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
  let added = ref(false);
  let target = ref([||]);
  let ret = {
    ...school,
    classrooms:
      Array.iter(r => /* Iterate through the classrooms, and add each one to the squashed ref */
                 if (Array.length(squashed^) == 0) {
                   target := Array.append(target, Array.make(1, classroom))
                 } else {
                   let already_included = Array.map(oldr => oldr.letter, school.classrooms);
                   let found = ref(false);
                   let idx = ref(0)/* This will only be read later if found is toggled to true*/;
                   Array.iter((i, l) =>
                              if (classroom.letter == l) {
                                found := true/* use iteri!!!!*/;
                                idx := i;
                              },
                              already_included);
                   if (found^) {
                     added := true /* Ideally we're not using this - there are three outcomes in this block already which should suffice */
                     Array.set(squashed^, idx, Array.append(Array.get(squashed^, idx), classroom))/* You need to mathc the classroom, but append to the kids*/;
                   }
                   )
                 };
                 ,school.classrooms)
      /* Instead of a fold, I think this is just an iter, and we build up mutably*/
      /*Array.fold_left(
    (target, oldr) =>
      if (Array.length(target) == 0) {
         /* The first time through, just add the first room as-is */ Array.append(target, Array.make(1, classroom))
       } else {
         /* Otherwise, check if we're a duplicate and then either pop it in or add the new kids to the existing entry */
        let already_included = Array.map(r => r.letter, school.classrooms);
        let found = ref(false)/* Once this works, see if we can make this all a little more functional */;
        Array.iter(/* Check through every room we've already added */ l => 
                   if (classroom.letter == l) {
          found := true;
        },
        already_included);
        if (found^) {
          added := true;
          oldr.kids := Array.append(oldr.kids^, Array.append(classroom.kids^, oldr.kids^));
          Array.append(target, [||])/* Nothing to add, we just merged the kids to an existing record */;
        } else {
          Array.append(target, [||])/* Can this match arm replace the added stuff below?*/;
        }
      },
    [||],
    school.classrooms,
       )*/
  };
  
  if (! added^) {
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
