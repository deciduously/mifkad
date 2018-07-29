/* App.re - main container */
open Belt;

/* payload types to match types in /src/schema.rs */

/* type expected = Core | Extended | Unscheduled; */

type schedule = {
  weekday: string,
  expected: string,
};

type kid = {
  name: string,
  schedule: array(schedule),
};

type classroom = {
  letter: string,
  capacity: int,
  kids: array(kid),
};

type school = { classrooms: array(classroom) };

/* ReasonReact types */
     
type state =
  | Loading
  | Error
  | Loaded(school);

type action =
  | GetEnrollment(string) /* This is the day */
  | EnrollmentReceived(school)
  | EnrollmentFailedToGet;

module Decode = {
  let schedule = json: schedule =>
    Json.Decode.({
    weekday: json |> field("weekday", string),
    expected: json |> field("expected", string)
  });

  let kid = json: kid =>
    Json.Decode.({
    name: json |> field("name", string),
    schedule: json |> field("schedule", array(schedule)) |> Array.map(_, schedule => schedule)
  });

  let classroom = json: classroom =>
    Json.Decode.({
    letter: json |> field("letter", string),
    capacity: json |> field("capacity", int),
    kids: json |> field("kids", array(kid)) |> Array.map(_, kid => kid)
  });

  let school = json: school =>
    Json.Decode.({
    classrooms: json |> field("classrooms", array(classroom)) |> Array.map(_, classroom => classroom)
 });
};

let component = ReasonReact.reducerComponent("App");

/* underscores indicate unused, like in Rust */
/* last one must be children */
let make = _children => {
  ...component, /* spread the template's other defaults into here - otherwise it'd be in a double [|[|<child>|]|] */
  initialState: _state => Loading,
  reducer: (action, _state) =>
    switch (action) {
    | GetEnrollment(s) =>
      ReasonReact.UpdateWithSideEffects(
      Loading,
      (
        self =>
          Js.Promise.(
        Fetch.fetch("http://127.0.0.1:8080/school/mon") /* TODO string inpterpolate s */
        |> then_(Fetch.Response.json)
        |> then_(json =>
                 json
                 |> Decode.school
                 |> (school => self.send(EnrollmentReceived(school)))
                 |> resolve
          )
        |> catch(_err =>
                 Js.Promise.resolve(self.send(EnrollmentFailedToGet))
          )
        |> ignore
      ) 
      ),
    )
    | EnrollmentReceived(school) => ReasonReact.Update(Loaded(school))
    | EnrollmentFailedToGet => ReasonReact.Update(Error)
    },
  didMount: self => self.send(GetEnrollment("mon")),
  render: self =>
    switch (self.state) {
    | Error => <div> (ReasonReact.string("OH NO AN ERROR")) </div>
    | Loading => <div> (ReasonReact.string("Loading...")) </div>
    | Loaded(school) =>
      let cname =
      switch (school.classrooms[0]) {
      | None => "No class found in mon"
      | Some(s) => s.letter
      };
        <div id="app">
          <h1>{ReasonReact.string("Attendance")}</h1>
          <hr />
            <FileConsole />
              {ReasonReact.string(cname)}
          <hr />
          <Roster roster="Placeholder 1" /> /* children */
          <hr />
          <Roster roster="RosterPlaceholder2" />
          <hr />
          <footer>
            {ReasonReact.string("\xA9 2018 deciduously - ")}
            <a href="https://github.com/deciduously/mifkad">{ReasonReact.string("source")}</a>
          </footer>
        </div>
        }
};
