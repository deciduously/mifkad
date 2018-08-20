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
        {
          core ?
            {
              let absent =
                Array.of_list(
                  List.filter(
                    k => !k.schedule.actual,
                    Array.to_list(room.kids^),
                  ),
                );
              <div>
                <span> {ReasonReact.string("Absent:")} </span>
                <ul>
                  {
                    Array.map(
                      k =>
                        <li key={k.name}> {ReasonReact.string(Report.to_fmt_name(k.name))} </li>,
                      absent,
                    )
                    |> ReasonReact.array
                  }
                </ul>
              </div>;
            } :
            <span />
        }
      </div>,
  };
};
