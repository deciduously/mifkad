/* ExtendedDay.re is responsible for configuring which core rooms go to which extended day rooms */
open Types;

type state = string;
let component = ReasonReact.reducerComponent("ExtendedDay");

let valueFromEvent = event => (event |> ReactEvent.Form.target)##value;

/* You need a list of each core classroom, and a dropdown next to each with each extended classroom
 *  Read the extended classrooms from the list passed in
 *  On change of dropdown, fire an event to update the config
 *  You also need to change the rendering part to use this config format instead of the mapping fn
 */

let make =
    (
      ~config: extended_config,
      ~addExtRoomClicked,
      ~removeExtRoomClicked,
      _children,
    ) => {
  ...component,
  initialState: () => "XE",
  reducer: (newText, _text) => ReasonReact.Update(newText),
  render: ({state: text, send}) =>
    <ul className="extendedday">
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
           config,
         ),
       )
       |> ReasonReact.array}
      <li>
        <input
          id="newextroom"
          value=text
          type_="text"
          placeholder="XE"
          onChange={event => send(valueFromEvent(event))}
          onKeyDown={event =>
            if (ReactEvent.Keyboard.key(event) == "Enter") {
              /* TODO - check for duplicate, check for empty */
              addExtRoomClicked(text);
              send("XE");
            }
          }
        />
      </li>
    </ul>,
};