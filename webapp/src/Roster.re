/* Roster.re is the top-level container for holding the roster */
let component = ReasonReact.statelessComponent("Roster");

let make = (~roster, _children) => {
  ...component,
  render: (self) => {
  <div id=roster>
    {ReasonReact.string(roster)}
    </div>
  }
}
