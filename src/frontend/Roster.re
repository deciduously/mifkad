/* Roster.re is the top-level container for holding the roster */
open Types;

let component = ReasonReact.statelessComponent("Roster");

let make =
    (
      ~school,
      ~kidClicked,
      ~addextClicked,
      ~collectedClicked,
      ~core,
      _children,
    ) => {
  ...component,
  render: _self =>
    <div className="roster">
        {
          Array.map(
            classroom =>
              <div key={classroom.letter} className="room">
                <Room
                  room=classroom
                  kidClicked
                  addextClicked
                  collectedClicked
                  core
                />
              </div>,
            school.classrooms,
          )
          |> ReasonReact.array
        }
    </div>,
};
