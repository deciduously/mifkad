/* App.re - main container */
open Belt;
open Types /* ReasonReact types */;

type state =
  | ChooseDay
  | Loading
  | Error
  | Loaded(school, string /* extended_config*/);

type action =
  | GetEnrollment(string) /* This is the day */
  | EnrollmentReceived(school)
  | EnrollmentFailedToGet
  | ResetDay /* Toggle carries a kid payload, not just a name? */
  | RoomCollected(school, classroom, string)
  | AddToExtended(school, kid, string)
  | ToggleExtendedConfig(school, string /* extended_config*/) /* Rigth now there are just two states, fall and summer. */
  | Toggle(school, kid, string);

module Decode = {
  let day = json: day =>
    Json.Decode.{
      expected: json |> field("expected", string),
      actual: json |> field("actual", bool),
    };

  let kid = json: kid =>
    Json.Decode.{
      id: json |> field("id", int),
      name: json |> field("name", string),
      schedule: json |> field("schedule", day),
    };

  let classroom = json: classroom =>
    Json.Decode.{
      id: json |> field("id", int),
      letter: json |> field("letter", string),
      capacity: json |> field("capacity", int),
      collected: json |> field("collected", bool),
      kids:
        ref(json |> field("kids", array(kid)) |> Array.map(_, kid => kid)),
    };

  let school = json: school =>
    Json.Decode.{
      weekday: json |> field("weekday", string),
      classrooms:
        json
        |> field("classrooms", array(classroom))
        |> Array.map(_, classroom => classroom),
    };
};

let component = ReasonReact.reducerComponent("App");

let make = _children => {
  ...component,
  initialState: _state => Loading,
  didMount: self => self.send(GetEnrollment("today")),
  reducer: (action, _state) =>
    switch (action) {
    | GetEnrollment(s) =>
      ReasonReact.UpdateWithSideEffects(
        Loading,
        (
          self =>
            Js.Promise.(
              Fetch.fetch("http://127.0.0.1:8080/school/" ++ s)
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
    | EnrollmentReceived(school) =>
      ReasonReact.Update(
        Loaded(school, "F8" /* TODO - how will this work */),
      )
    | EnrollmentFailedToGet => ReasonReact.Update(Error)
    | ResetDay => ReasonReact.Update(ChooseDay)
    | RoomCollected(school, room, extended_config) =>
      ReasonReact.UpdateWithSideEffects(
        Loaded(toggle_collected(school, room), extended_config) /* Assume it will work and flip in the frontend */,
        (
          self => {
            Js.Promise.(
              Fetch.fetch("http://127.0.0.1:8080/collected/" ++ string_of_int(room.id))
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
            );
          }
        ),
      )
    | ToggleExtendedConfig(school, extended_config) =>
      ReasonReact.Update(
        Loaded(school, extended_config == "M8" ? "F8" : "M8"),
      )
    | AddToExtended(school, kid, extended_config) =>
      ReasonReact.UpdateWithSideEffects(
        Loaded(toggle_extended(school, kid), extended_config) /* Assume it will work and flip in the frontend */,
        (
          self => {
            Js.Promise.(
              Fetch.fetch("http://127.0.0.1:8080/addext/" ++ string_of_int(kid.id))
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
            );
          }
        ),
      )
    | Toggle(school, kid, extended_config) =>
    ReasonReact.UpdateWithSideEffects(
      Loaded(toggle(school, kid), extended_config) /* Assume it will work and flip in the frontend */,
      (
        self => {
          Js.Promise.(
            Fetch.fetch("http://127.0.0.1:8080/toggle/" ++ string_of_int(kid.id))
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
          );
        }
      ),
    )
  },
  render: self =>
    switch (self.state) {
    | ChooseDay =>
      <div>
        <h2> {ReasonReact.string("Please select day:")} </h2>
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
    | Error =>
      <div>
        {
          ReasonReact.string(
            "An error occured connecting to the backend.  Check the server log.",
          )
        }
      </div>
    | Loading => <div> {ReasonReact.string("Loading...")} </div>
    | Loaded(school, extended_config) =>
      <div id="app">
        <h1> {ReasonReact.string("Mifkad")} </h1>
        <h2> {ReasonReact.string("Attendance - " ++ school.weekday)} </h2>
        <hr />
        <OutputViewer school extended_config />
        <hr />
        <Roster
          school
          kidClicked=(
            event => self.send(Toggle(school, event, extended_config))
          )
          addextClicked=(
            event => self.send(AddToExtended(school, event, extended_config))
          )
          collectedClicked=(
            event => self.send(RoomCollected(school, event, extended_config))
          )
          core=true
        /> /* core=true means it'll render toggleable buttons */
        <hr />
        <Roster
          school={get_extended_rooms(school, extended_config)}
          kidClicked=(_event => ())
          addextClicked=(_event => ())
          collectedClicked=(_event => ())
          core=false
        />
        <hr />
        <span>
          {
            ReasonReact.string(
              "Extended day config: "
              ++ (extended_config == "M8" ? "summer" : "fall"),
            )
          }
        </span>
        <br />
        <button
          onClick=(
            _event =>
              self.send(ToggleExtendedConfig(school, extended_config))
          )>
          {
            ReasonReact.string(
              "Switch to " ++ (extended_config == "M8" ? "fall" : "summer"),
            )
          }
        </button>
        {ReasonReact.string(" ")}
        <button onClick=(_event => self.send(ResetDay))>
          {ReasonReact.string("Pick a different day")}
        </button>
        <footer>
          <hr />
          {ReasonReact.string("mifkad v0.1.3 \xA9 2018 Ben Lovy - ")}
          <a href="https://github.com/deciduously/mifkad">
            {ReasonReact.string("source")}
          </a>
        </footer>
      </div>
    },
};
