/* Room.re renders a single room */
open Types;

let component = ReasonReact.statelessComponent("Room");

let make =
    (
      ~room: classroom,
      ~kidClicked,
      ~addextClicked,
      ~collectedClicked,
      ~core,
      _children,
    ) => {
  let click = _event => collectedClicked(room);
  {
    ...component,
    render: _self => {
      let headcount = Array.length(room.kids^);
      let absent =
        Array.of_list(
          List.filter(k => !k.schedule.actual, Array.to_list(room.kids^)),
        );
      <div className="roomContent">
        <h4 className="roomLetter">
          {ReasonReact.string("Room " ++ room.letter)}
        </h4>
        <p>
          {
            ReasonReact.string(
              string_of_int(headcount - Array.length(absent))
              ++ "/"
              ++ string_of_int(headcount)
              ++ " - max "
              ++ string_of_int(room.capacity),
            )
          }
          <br />
          {
            core ?
              <button onClick=click>
                {
                  ReasonReact.string(
                    room.collected ? "Ready" : "Need to finish",
                  )
                }
              </button> :
              <span />
          }
        </p>
        <KidList kids=room.kids^ kidClicked addextClicked core />
        {
          core ?
            <div>
              <span> {ReasonReact.string("Absent:")} </span>
              <ul>
                {
                  Array.map(
                    k =>
                      <li key={k.name}>
                        {ReasonReact.string(Report.to_fmt_name(k.name))}
                      </li>,
                    absent,
                  )
                  |> ReasonReact.array
                }
              </ul>
            </div> :
            <span />
        }
      </div>;
    },
  };
};
