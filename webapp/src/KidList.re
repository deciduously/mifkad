/* Kidlist.re is responsible for rendering the list of kids in a room */
open Types;

let component = ReasonReact.statelessComponent("KidList");

let make = (~kids, _children) => {
  ...component,
  render: _self => {
  <ul className="kidlist">
    <li className="capacity">{ReasonReact.string("Capacity")}</li>
    <Kid kid=kids[0] />
</ul>
}
}
