/* ExtendedDay.re is responsible for configuring which core rooms go to which extended day rooms */
/* For now to change a room capacity you need to delete it and recreate it with the right number */
open Types;

type state = (string, string) /* new letter, new capacity */;
type action =
  | ChangeLetter(string)
  | ChangeCapacity(string)
  | Reset;

let default_state = ("XE", "20");

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
  initialState: () => default_state,
  reducer: (action, state) =>
    switch (action) {
    | ChangeLetter(letter) => ReasonReact.Update((letter, snd(state)))
    | ChangeCapacity(capacity) => ReasonReact.Update((fst(state), capacity))
    | Reset => ReasonReact.Update(default_state)
    },
  render: self =>
    <div id="extroomconfig">
      <div id="extroomlist">
        <ul>
          {
            Array.of_list(
              List.map(
                (extroom: extended_day_entry) => {
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
            |> ReasonReact.array
          }
          <li>
            <input
              id="newextroom"
              value={fst(self.state)}
              type_="text"
              placeholder="XE"
              onChange={
                event =>
                  self.send(
                    ChangeLetter((event |> ReactEvent.Form.target)##value),
                  )
              }
              onKeyDown={
                event =>
                  if (ReactEvent.Keyboard.key(event) == "Enter") {
                    addExtRoomClicked(self.state);
                    self.send(Reset);
                  }
              }
            />
            <input
              id="newextcap"
              value={snd(self.state)}
              type_="text"
              placeholder="20"
              onChange={
                event =>
                  self.send(
                    ChangeCapacity((event |> ReactEvent.Form.target)##value),
                  )
              }
              onKeyDown={
                event =>
                  if (ReactEvent.Keyboard.key(event) == "Enter") {
                    addExtRoomClicked(self.state);
                    self.send(Reset);
                  }
              }
            />
          </li>
        </ul>
      </div>
      <div id="coreconfig">
        <ul>
          {
            Array.map(
              (classroom: classroom) =>
                <li key={classroom.letter}>
                  {ReasonReact.string(classroom.letter ++ " => ")}
                  <select
                    value={
                      get_extended_letter(
                        classroom.letter,
                        school.extended_day_config,
                      )
                    }
                    onChange={
                      event =>
                        adjExtRoomFired(
                          classroom.letter,
                          (event |> ReactEvent.Form.target)##value,
                        )
                    }>
                    {
                      Array.of_list(
                        List.append(
                          [
                            <option key="unassignedopt" value="Unassigned">
                              {ReasonReact.string("Unassigned")}
                            </option>,
                          ],
                          List.map(
                            (extroom: extended_day_entry) =>
                              <option
                                key={extroom.letter ++ "opt"}
                                value={extroom.letter}>
                                {ReasonReact.string(extroom.letter)}
                              </option>,
                            Array.to_list(school.extended_day_config.entries),
                          ),
                        ),
                      )
                      |> ReasonReact.array
                    }
                  </select>
                </li>,
              school.classrooms,
            )
            |> ReasonReact.array
          }
        </ul>
      </div>
      <div>
        <button onClick=saveExtConfigClicked>
          {ReasonReact.string("Save Config")}
        </button>
      </div>
    </div>,
};
