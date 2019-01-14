/* Types.re holds types and transformations used throughout the UI components */

/* payload types to match types in /src/schema.rs */

/* TODO type expected = Core | Extended | Unscheduled; */

open Belt.Option;

/* External JS functions */
[@bs.val] external alert: string => unit = "alert";
[@bs.val] external confirm: string => bool = "confirm";
[@bs.val] external btoa: string => string = "btoa";

/* Type declarations */
type day = {
  expected: string,
  actual: bool,
};

type kid = {
  id: int,
  name: string,
  schedule: day,
};

type classroom = {
  id: int,
  letter: string,
  capacity: int,
  collected: bool,
  kids: ref(array(kid)),
};

type extended_day_entry = {
  letter: string,
  capacity: string /* for simplicity's sake - its only cosmetic anyway */,
  members: array(string),
};

type extended_day_config = {entries: array(extended_day_entry)};

type school = {
  classrooms: array(classroom),
  extended_day_config,
  weekday: string,
};

let get_uncollected_rooms = school =>
  /* Returns the letters of rooms havent been marked complete as a string */
  school.classrooms
  |> Array.to_list
  |> List.filter(r => !r.collected)
  |> List.fold_left((s, r: classroom) => s ++ r.letter ++ " ", "");

let new_extended_day_entry = (letter, capacity) => {
  letter,
  capacity,
  members: [||],
};

let contains = (list, target) =>
  /* Does target appear in list? */
  List.fold_left((acc, el) => acc || el == target, false, list);

let add_extended_letter =
    (letter, capacity, extended_config): extended_day_config =>
  if (letter == "") {
    alert("New class name was empty!");
    extended_config;
  } else if (letter |> Js.String.match([%re "/^[A-Z]E$/"]) |> isNone) {
    alert("Must use a single capital letter followed by E, like XE or BE");
    extended_config;
  } else if (contains(
               List.map(
                 el => el.letter,
                 Array.to_list(extended_config.entries),
               ),
               letter,
             )) {
    alert("Already exists!");
    extended_config;
  } else {
    {
      entries:
        Array.append(
          Array.make(1, new_extended_day_entry(letter, capacity)),
          extended_config.entries,
        )
        |> Array.to_list
        |> List.sort(compare)
        |> Array.of_list,
    };
  };

let remove_extended_letter = (letter, extended_config) => {
  entries:
    Array.of_list(
      List.filter(
        entry => entry.letter != letter,
        Array.to_list(extended_config.entries),
      ),
    ),
};

let get_extended_capacity = (letter, extended_config) => {
  let matches =
    List.filter(
      entry => entry.letter == letter,
      Array.to_list(extended_config.entries),
    );
  List.length(matches) > 0 ? List.hd(matches).capacity : "0";
};

let get_extended_letter = (letter, extended_config) => {
  /* Grab the extended letter from the config */
  let matches =
    List.filter(
      entry => contains(Array.to_list(entry.members), letter),
      Array.to_list(extended_config.entries),
    );
  List.length(matches) > 0 ? List.hd(matches).letter : "Unassigned";
};

let adjust_extended_config = (letter, target, extended_config) =>
  /* Returns a new extended_day_config with "letter" removed from existing entry and added to target entry */
  {
    entries:
      List.map(
        entry =>
          if (contains(Array.to_list(entry.members), letter)) {
            {
              letter: entry.letter,
              capacity: entry.capacity,
              members:
                Array.of_list(
                  List.filter(
                    el => el != letter,
                    Array.to_list(entry.members),
                  ),
                ),
            };
          } else if (entry.letter == target) {
            {
              letter: entry.letter,
              capacity: entry.capacity,
              members: Array.append([|letter|], entry.members),
            };
          } else {
            entry;
          },
        Array.to_list(extended_config.entries),
      )
      |> Array.of_list,
  };

let get_extended_kids = school =>
  /* Returns a school with only the Extended Day kids */
  {
    ...school,
    classrooms:
      Array.map(
        (r: classroom) => {
          ...r,
          letter: get_extended_letter(r.letter, school.extended_day_config),
          kids:
            ref(
              Array.of_list(
                List.filter(
                  k =>
                    k.schedule.expected == "Extended"
                    || k.schedule.expected == "Added",
                  Array.to_list(r.kids^),
                ),
              ),
            ),
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
    target := Array.append(target^, Array.make(1, classroom));
  } else {
    let already_included =
      Array.map((oldr: classroom) => oldr.letter, school.classrooms);
    let found = ref(false);
    let idx = ref(0) /* This will only be read later if found is toggled to true*/;
    Array.iteri(
      (i, l) =>
        if (classroom.letter == l) {
          found := true;
          idx := i;
        },
      already_included,
    );
    if (found^) {
      /* We've already seen this letter - mash the new kid list into the matching existing kid list */
      let old_classroom = school.classrooms[idx^];
      let new_classroom = {
        ...old_classroom,
        capacity:
          get_extended_capacity(classroom.letter, school.extended_day_config)
          |> int_of_string,
        kids: ref(Array.append(old_classroom.kids^, classroom.kids^)),
      };
      target^[idx^] = new_classroom;
    } else {
      /* This is a new extended day room - add it as-is, grabbing the extended day capacity */
      target :=
        Array.append(
          target^,
          Array.make(
            1,
            {
              ...classroom,
              capacity:
                get_extended_capacity(
                  classroom.letter,
                  school.extended_day_config,
                )
                |> int_of_string,
            },
          ),
        );
    };
  };

  {...school, classrooms: target^};
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
          ref(
            Array.map(
              k =>
                if (kid == k) {
                  {
                    ...k,
                    schedule: {
                      ...k.schedule,
                      actual: !k.schedule.actual,
                    },
                  };
                } else {
                  k;
                },
              room.kids^,
            ),
          ),
      },
      school.classrooms,
    ),
};

let toggle_extended = (school, kid) => {
  /* Returns a new school with the specified kid toggled */
  ...school,
  classrooms:
    Array.map(
      room => {
        ...room,
        kids:
          ref(
            Array.map(
              k =>
                if (kid == k) {
                  {
                    ...kid,
                    schedule: {
                      ...kid.schedule,
                      expected:
                        kid.schedule.expected == "Core" ? "Added" : "Core",
                    },
                  };
                } else {
                  k;
                },
              room.kids^,
            ),
          ),
      },
      school.classrooms,
    ),
};

let toggle_collected = (school, classroom) => {
  /* Returns a new school with the specified classroom toggled */
  ...school,
  classrooms:
    Array.map(
      room =>
        if (room == classroom) {
          {...room, collected: !room.collected};
        } else {
          room;
        },
      school.classrooms,
    ),
};
