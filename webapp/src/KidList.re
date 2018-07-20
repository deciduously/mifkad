/* Kidlist.re is responsible for rendering the list of kids in a room */
let component = ReasonReact.statelessComponent("KidList");

let make = (~kids, _children) => {
  ...component,
  render: _self => {
  <ul>
    <li>{ReasonReact.string("Capacity")}</li>
    <li>{ReasonReact.string("Placeholder Kid")}</li>
</ul>
}
}
