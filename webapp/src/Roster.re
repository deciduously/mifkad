/* Roster.re is the top-level container for holding the roster */
open Types;

let component = ReasonReact.statelessComponent("Roster");

let make = (~roster, _children) => {
  ...component,
  render: _self => {
  let classname = roster.classrooms[0].letter;
  <div className="roster">
    <ul className="classList"><Room room=roster.classrooms[0] /> </ul>
    {ReasonReact.string(classname)}
    </div>
  }
}
