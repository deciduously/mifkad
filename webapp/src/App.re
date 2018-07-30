/* App.re - main container */
open Belt;
open Types;

/* ReasonReact types */
     
type state =
  | ChooseDay
  | Loading
  | Error
  | Loaded(school);

type action =
  | GetEnrollment(string) /* This is the day */
  | EnrollmentReceived(school)
  | EnrollmentFailedToGet
  | ResetDay
  | Toggle(school);

module Decode = {
  let day = json: day =>
    Json.Decode.({
    weekday: json |> field("weekday", string),
    expected: json |> field("expected", string),
    actual: json |> field("actual", bool)
  });

  let kid = json: kid =>
    Json.Decode.({
    name: json |> field("name", string),
    schedule: json |> field("schedule", day)
  });

  let classroom = json: classroom =>
    Json.Decode.({
    letter: json |> field("letter", string),
    capacity: json |> field("capacity", int),
    kids: json |> field("kids", array(kid)) |> Array.map(_, kid => kid)
  });

  let school = json: school =>
    Json.Decode.({
    weekday: json |> field("weekday", string),
    classrooms: json |> field("classrooms", array(classroom)) |> Array.map(_, classroom => classroom)
 });
};

let component = ReasonReact.reducerComponent("App");

/* underscores indicate unused, like in Rust */
/* last one must be children */
let make = _children => {
  ...component, /* spread the template's other defaults into here - otherwise it'd be in a double [|[|<child>|]|] */
  initialState: _state => ChooseDay,
  reducer: (action, _state) =>
    switch (action) {
    | GetEnrollment(s) =>
      ReasonReact.UpdateWithSideEffects(
      Loading,
      (
        self =>
          Js.Promise.(
        Fetch.fetch("http://127.0.0.1:8080/school/" ++ s) /* TODO string inpterpolate s */
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
    | ResetDay => ReasonReact.Update(ChooseDay)
    | Toggle(school) => ReasonReact.Update(Loaded(school))
    },
  /* didMount: self => self.send(GetEnrollment("tue")), */ /* We don't need to do this, user chooses day first */
  render: self =>
    switch (self.state) {
    | ChooseDay =>
      <div>
        <h2> (ReasonReact.string("Please select day:")) </h2>
        <br />
        <div>
          <button onClick=(_event => self.send(GetEnrollment("mon")))>
            {ReasonReact.string("Monday")}
           </button>
           <button onClick=(_event => self.send(GetEnrollment("tue")))>
              {ReasonReact.string("Tuesday")}
           </button>
          <button onClick=(_event => self.send(GetEnrollment("wed")))>
            {ReasonReact.string("Wednesday")}
          </button>
          <button onClick=(_event => self.send(GetEnrollment("thu")))>
            {ReasonReact.string("Thursday")}
          </button>
          <button onClick=(_event => self.send(GetEnrollment("fri")))>
            {ReasonReact.string("Friday")}
          </button>
        </div>
      </div>
    | Error => <div> (ReasonReact.string("An error occured connecting to the backend.  Check the server log.")) </div>
    | Loading => <div> (ReasonReact.string("Loading...")) </div>
    | Loaded(school) =>
        <div id="app">
          <h1>
              (ReasonReact.string("Attendance - " ++ school.weekday))
          </h1>
          <hr />
          <FileConsole onClick=(_event => self.send(ResetDay))/>
          <hr />
          <Roster school=school onClick=(_event => self.send(Toggle(school))) />
          <hr />
          /*<Roster roster="RosterPlaceholder2" />*/
          <hr />
          <footer>
            (ReasonReact.string("\xA9 2018 deciduously - "))
            <a href="https://github.com/deciduously/mifkad">
                (ReasonReact.string("source"))
            </a>
          </footer>
        </div>
        }
};
