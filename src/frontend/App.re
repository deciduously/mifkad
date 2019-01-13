/* App.re - main container */
open Belt;
open Types;

type state =
  | Loading
  | Error
  | Loaded(school);

type action =
  | GetEnrollment(string) /* This is the day */
  | EnrollmentReceived(school)
  | EnrollmentFailedToGet
  | Reset /* Clear attendance data */
  | RoomCollected(school, classroom)
  | AdjExtRoom(string, string, school)
  | AddToExtended(school, kid)
  | AddExtRoom(school, (string, int))
  | RemoveExtRoom(school, string)
  | SaveExtConfig(school)
  | Toggle(school, kid);

module Decode = {
  let day = (json): day =>
    Json.Decode.{
      expected: json |> field("expected", string),
      actual: json |> field("actual", bool),
    };

  let kid = (json): kid =>
    Json.Decode.{
      id: json |> field("id", int),
      name: json |> field("name", string),
      schedule: json |> field("schedule", day),
    };

  let classroom = (json): classroom =>
    Json.Decode.{
      id: json |> field("id", int),
      letter: json |> field("letter", string),
      capacity: json |> field("capacity", int),
      collected: json |> field("collected", bool),
      kids:
        ref(json |> field("kids", array(kid)) |> Array.map(_, kid => kid)),
    };

  let extended_day_entry = (json): extended_day_entry =>
    Json.Decode.{
      letter: json |> field("letter", string),
      capacity: json |> field("capacity", int),
      members:
        json
        |> field("members", array(string))
        |> Array.map(_, member => member),
    };

  let extended_day_config = (json): extended_day_config =>
    Json.Decode.{
      entries:
        json
        |> field("entries", array(extended_day_entry))
        |> Array.map(_, extended_day_entry => extended_day_entry),
    };

  let school = (json): school =>
    Json.Decode.{
      weekday: json |> field("weekday", string),
      extended_day_config:
        json |> field("extended_day_config", extended_day_config),
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
          ),
      )
    | EnrollmentReceived(school) => ReasonReact.Update(Loaded(school))
    | EnrollmentFailedToGet => ReasonReact.Update(Error)
    | Reset =>
      ReasonReact.UpdateWithSideEffects(
        Loading,
        self =>
          Js.Promise.(
            Fetch.fetch(
              "http://127.0.0.1:8080/reset/0" /* Trailing zero is ignored by the backend */,
            )
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
          ),
      )
    | RoomCollected(school, room) =>
      ReasonReact.UpdateWithSideEffects(
        Loaded(toggle_collected(school, room)) /* Assume it will work and flip in the frontend */,
        self =>
          Js.Promise.(
            Fetch.fetch(
              "http://127.0.0.1:8080/collect/" ++ string_of_int(room.id),
            )
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
          ),
      )
    | AddToExtended(school, kid) =>
      ReasonReact.UpdateWithSideEffects(
        Loaded(toggle_extended(school, kid)) /* Assume it will work and flip in the frontend */,
        self =>
          Js.Promise.(
            Fetch.fetch(
              "http://127.0.0.1:8080/addext/" ++ string_of_int(kid.id),
            )
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
          ),
      )
    | AdjExtRoom(letter, target, school) =>
      ReasonReact.Update(
        Loaded({
          ...school,
          extended_day_config:
            adjust_extended_config(
              letter,
              target,
              school.extended_day_config,
            ),
        }),
      )
    | AddExtRoom(school, (letter, capacity)) =>
      ReasonReact.Update(
        Loaded({
          ...school,
          extended_day_config:
            add_extended_letter(letter, capacity, school.extended_day_config),
        }),
      )
    | RemoveExtRoom(school, extroom) =>
      ReasonReact.Update(
        Loaded({
          ...school,
          extended_day_config:
            remove_extended_letter(extroom, school.extended_day_config),
        }),
      )
    | SaveExtConfig(school) =>
      alert("Saving...");
      ReasonReact.Update(Loaded(school));
    | Toggle(school, kid) =>
      ReasonReact.UpdateWithSideEffects(
        Loaded(toggle(school, kid)) /* Assume it will work and flip in the frontend */,
        self =>
          Js.Promise.(
            Fetch.fetch(
              "http://127.0.0.1:8080/toggle/" ++ string_of_int(kid.id),
            )
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
          ),
      )
    },
  render: self =>
    switch (self.state) {
    | Error =>
      <div>
        {ReasonReact.string(
           "An error occured connecting to the backend.  Check the server log.",
         )}
        <br />
        <button onClick={_event => self.send(GetEnrollment("today"))}>
          {ReasonReact.string("Try to reconnect")}
        </button>
      </div>
    | Loading => <div> {ReasonReact.string("Loading...")} </div>
    | Loaded(school) =>
      <div id="app">
        <h1> {ReasonReact.string("Mifkad")} </h1>
        <h2> {ReasonReact.string("Attendance - " ++ school.weekday)} </h2>
        <hr />
        <OutputViewer
          school
          refreshClicked={_event => self.send(GetEnrollment("today"))}
          resetClicked={_event => self.send(Reset)}
        />
        <hr />
        <Roster
          school
          kidClicked={event => self.send(Toggle(school, event))}
          addextClicked={event => self.send(AddToExtended(school, event))}
          collectedClicked={event => self.send(RoomCollected(school, event))}
          core=true
        /> /* core=true means it'll render toggleable buttons */
        <hr />
        <Roster
          school={get_extended_rooms(school)}
          kidClicked={_event => ()}
          addextClicked={_event => ()}
          collectedClicked={_event => ()}
          core=false
        />
        <hr />
        <ExtendedDay
          school
          adjExtRoomFired={(letter, target) =>
            self.send(AdjExtRoom(letter, target, school))
          }
          addExtRoomClicked={event => self.send(AddExtRoom(school, event))}
          saveExtConfigClicked={_event => self.send(SaveExtConfig(school))}
          removeExtRoomClicked={event =>
            self.send(RemoveExtRoom(school, event))
          }
        />
        <footer>
          <hr />
          {ReasonReact.string("mifkad v0.2.4 \xA9 2019 Ben Lovy - ")}
          <a href="https://github.com/deciduously/mifkad">
            {ReasonReact.string("source")}
          </a>
        </footer>
      </div>
    },
};