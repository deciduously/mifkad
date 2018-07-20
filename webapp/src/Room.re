/* Room.re renders a single room */
let component = ReasonReact.statelessComponent("Room");

let make = (~room, _children) => {
  ...component,
  render: _self => {
  <li>
    <div><h4>{ReasonReact.string(room)}</h4><KidList kids="Placeholder Kids" /></div>
</li>
}
}
