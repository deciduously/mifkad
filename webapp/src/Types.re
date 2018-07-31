/* Types.re holds types used throughout */

/* payload types to match types in /src/schema.rs */

/* type expected = Core | Extended | Unscheduled; */

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
  weekday: string
};

/* Returns a school with the given name toggled */
let toggle = (school, kid) =>
  {...school,
   classrooms: Array.map(room =>
                         {...room,
                          kids: Array.map(k =>
                                          if (kid == k) {
                                            {...kid,
                                             schedule: {...kid.schedule,
                                                        actual: !kid.schedule.actual } }
                                          } else {
                                            k
                                          }
                                          , room.kids)},
                         school.classrooms)};

/* Returns a single string reporting the school's attendance */
let report = school =>
  Array.fold_left((acc, room) =>
                  acc ++ " " ++
                  room.letter ++
                  Array.fold_left((inner_acc, kid) =>
                                  inner_acc ++ " - " ++ kid.name ++ ": " ++ (kid.schedule.actual ? "Here" : "Absent"),
                                  "",
                                  room.kids)
                  ++ "\n",
                  "",
                  school.classrooms)
