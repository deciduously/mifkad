/* Room.re renders a single room */
open Types;

let component = ReasonReact.statelessComponent("Room");

let make = (~room: classroom, ~onClick, ~core, _children) => {
  ...component,
  render: _self =>
    <div className="roomContent">
      <h4 className="roomLetter">
        {
          ReasonReact.string(
            room.letter ++ ": max " ++ string_of_int(room.capacity),
          )
        }
      </h4>
      <KidList kids=room.kids^ onClick core />
    </div>,
};
