/* FileConsole.re contains the buttons for roster uploading */
type state = {
   clicked: int,
};

type action = Click;

/* this needs to go after state and action */
let component = ReasonReact.reducerComponent("FileConsole");

let make = (_children) => {
  ...component,
  initialState: () => { clicked: 0 },
  reducer: (action, state) =>
      switch (action) {
        | Click => ReasonReact.Update({clicked: state.clicked + 1})
      },
  render: self => {
  <div id="fileconsole">
    <button id="console" onClick=(_event => self.send(Click))>
      {ReasonReact.string("Clicked " ++ string_of_int(self.state.clicked) ++ " times")}
    </button>
    <FileUpload id="roster" filetype="csv"/>
    </div>
  }
}
