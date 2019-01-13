/* ExtendedDay.re is responsible for configuring which core rooms go to which extended day rooms */
open Types;

type state = string;
let component = ReasonReact.reducerComponent("ExtendedDay");

let make =
    (
      ~school,
      ~adjExtRoomFired,
      ~addExtRoomClicked,
      ~saveExtConfigClicked,
      ~removeExtRoomClicked,
      _children,
    ) => {
  ...component,
  initialState: () => "XE",
  reducer: (newText, _text) => ReasonReact.Update(newText),
  render: ({state: text, send}) =>
    <div id="extroomconfig">
      <div id="extroomlist">
        <ul>
          {Array.of_list(
             List.map(
               extroom => {
                 let extroom_letter = extroom.letter;
                 <li key=extroom_letter>
                   {ReasonReact.string(extroom_letter)}
                   <button
                     onClick={_event => removeExtRoomClicked(extroom_letter)}>
                     {ReasonReact.string("Remove")}
                   </button>
                 </li>;
               },
               Array.to_list(school.extended_day_config.entries),
             ),
           )
           |> ReasonReact.array}
          <li>
            <input
              id="newextroom"
              value=text
              type_="text"
              placeholder="XE"
              onChange={event =>
                send((event |> ReactEvent.Form.target)##value)
              }
              onKeyDown={event =>
                if (ReactEvent.Keyboard.key(event) == "Enter") {
                  addExtRoomClicked(text);
                  send("XE");
                }
              }
            />
          </li>
        </ul>
      </div>
      <div id="coreconfig">
        <ul>
          {Array.map(
             (classroom: classroom) =>
               <li key={classroom.letter}>
                 {ReasonReact.string(classroom.letter ++ " => ")}
                 <select
                   value={get_extended_letter(
                     classroom.letter,
                     school.extended_day_config,
                   )}
                   onChange={event =>
                     adjExtRoomFired(
                       classroom.letter,
                       (event |> ReactEvent.Form.target)##value,
                     )
                   }>
                   {Array.of_list(
                      List.append(
                        [
                          <option key="unassignedopt" value="Unassigned">
                            {ReasonReact.string("Unassigned")}
                          </option>,
                        ],
                        List.map(
                          extroom =>
                            <option
                              key={extroom.letter ++ "opt"}
                              value={extroom.letter}>
                              {ReasonReact.string(extroom.letter)}
                            </option>,
                          Array.to_list(school.extended_day_config.entries),
                        ),
                      ),
                    )
                    |> ReasonReact.array}
                 </select>
               </li>,
             school.classrooms,
           )
           |> ReasonReact.array}
        </ul>
      </div>
      <div>
        <button onClick=saveExtConfigClicked>
          {ReasonReact.string("Save Config")}
        </button>
      </div>
    </div>,
};