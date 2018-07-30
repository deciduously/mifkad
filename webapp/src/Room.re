/* Room.re renders a single room */
open Types;

let component = ReasonReact.statelessComponent("Room");

let make = (~room : classroom, _children) => {
  ...component,
  render: _self => {
  <li key=room.letter className="room">
    <div className="roomContent">
      <h4 className="roomLetter">
          {ReasonReact.string(room.letter)}
      </h4>
      <KidList kids=room.kids />
    </div>
  </li>
  }
}
