/* Roster.re is the top-level container for holding the roster */
let component = ReasonReact.statelessComponent("Roster");

let make = (~roster, _children) => {
  ...component,
  render: _self => {
  <div id=roster>
    <ul><Room room="Placeholder Class" /> </ul>
    {ReasonReact.string(roster)}
    </div>
  }
}
