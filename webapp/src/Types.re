/* Types.re holds types used throughout */

/* payload types to match types in /src/schema.rs */

/* type expected = Core | Extended | Unscheduled; */

/* Type declarations */
/* cool - these are completely optimized away in the compiled js */
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

/* TODO */
let get_extended = school => school;

let toggle = (school, kid) => {
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
} /* Jury's out on which is superior */ /* One difference is that had a fn to only grab the absent kids, here I just didn't display those */ /* found at https://github.com/deciduously/attendance/blob/master/src/cljs/attendance/report.cljs */ /* Written to match output from the original ClojureScript version */ /* Returns a single string reporting the school's attendance */;

module Report = {
  /* Takes FIRSTNAME LASTNAME and returns Firstname L. */
  let to_fmt_name = name => {
    let idx_of_spc = String.index(name, ' ');

    let first_name =
      String.sub(name, 0, idx_of_spc) /* start_idx, len */
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
    ++ ": " /* Check if empty, and if it's not empty, trim off the trialing comma */
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
