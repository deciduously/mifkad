/* ExtendedDay.re is responsible for configuring which core rooms go to which extended day rooms */
open Types;

type state = string;
let component = ReasonReact.reducerComponent("ExtendedDay");

/* You need a list of each core classroom, and a dropdown next to each with each extended classroom
 *  Read the extended classrooms from the list passed in
 *  On change of dropdown, fire an event to update the config
 *  You also need to change the rendering part to use this config format instead of the mapping fn
 */

let make =
    (
      ~school: school,
      ~extended_config: extended_config,
      ~adjExtRoomFired,
      ~addExtRoomClicked,
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
                 let extroom_letter = fst(extroom);
                 <li key=extroom_letter>
                   {ReasonReact.string(extroom_letter)}
                   <button
                     onClick={_event => removeExtRoomClicked(extroom_letter)}>
                     {ReasonReact.string("Remove")}
                   </button>
                 </li>;
               },
               extended_config,
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
             classroom =>
               <li key={classroom.letter}>
                 {ReasonReact.string(classroom.letter ++ " => ")}
                 <select
                   value={get_extended_letter(
                     classroom.letter,
                     extended_config,
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
                              key={fst(extroom) ++ "opt"}
                              value={fst(extroom)}>
                              {ReasonReact.string(fst(extroom))}
                            </option>,
                          extended_config,
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
    </div>,
};