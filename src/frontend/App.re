/* App.re - main container */
open Belt;
open Types /* ReasonReact types */;

type state =
  | Loading
  | Error
  | Loaded(school, string /* extended_config*/);

type action =
  | GetEnrollment(string) /* This is the day */
  | EnrollmentReceived(school)
  | EnrollmentFailedToGet
  | Reset /* Clear attendance data */
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
    | Reset =>
    ReasonReact.UpdateWithSideEffects(
        Loading,
        (
          self => {
            Js.Promise.(
              Fetch.fetch("http://127.0.0.1:8080/reset/0"/* Trailing zero is ignored by the backend */)
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
    | RoomCollected(school, room, extended_config) =>
    ReasonReact.UpdateWithSideEffects(
      Loaded(toggle_collected(school, room), extended_config) /* Assume it will work and flip in the frontend */,
      (
        self => {
          Js.Promise.(
            Fetch.fetch("http://127.0.0.1:8080/collect/" ++ string_of_int(room.id))
            |> then_(Fetch.Response.json)
            |> then_(json =>
                 json
                 |> Decode.school
                 |> (school => self.send(EnrollmentReceived(school)))
                 |> resolve
               )
            |> catch(_err =>
                 Js.Promise.resolve(self.send(EnrollmentReceived(school)))
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
                   Js.Promise.resolve(self.send(EnrollmentReceived(school)))
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
                 Js.Promise.resolve(self.send(EnrollmentReceived(school)))
               )
            |> ignore
          );
        }
      ),
    )
  },
  render: self =>
    switch (self.state) {
    | Error =>
      <div>
        {
          ReasonReact.string(
            "An error occured connecting to the backend.  Check the server log.",
          )
        }
        <br/>
        <button onClick=(_event => self.send(GetEnrollment("today")))>{ReasonReact.string("Try to reconnect")}</button>
      </div>
    | Loading => <div> {ReasonReact.string("Loading...")} </div>
    | Loaded(school, extended_config) =>
      <div id="app">
        <h1> {ReasonReact.string("Mifkad")} </h1>
        <h2> {ReasonReact.string("Attendance - " ++ school.weekday)} </h2>
        <hr />
        <OutputViewer school extended_config refreshClicked=(_event => self.send(GetEnrollment("today"))) resetClicked=(_event => self.send(Reset))/>
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
