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
