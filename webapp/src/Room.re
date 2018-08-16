/* Room.re renders a single room */
open Types;

let component = ReasonReact.statelessComponent("Room");

let make =
    (~room: classroom, ~kidClicked, ~collectedClicked, ~core, _children) => {
  let click = _event => collectedClicked(room);
  {
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
        {
          core ?
            <button onClick=click>
              {
                ReasonReact.string(room.collected ? "Ready" : "Need to finish")
              }
            </button> :
            <span />
        }
        <KidList kids=room.kids^ onClick=kidClicked core />
      </div>,
  };
};
