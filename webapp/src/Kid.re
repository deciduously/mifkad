/* State declaration */
type state = {
  present: bool,
};

/* Action declaration */
type action =
  | Toggle;

/* Component template declaration.
   Needs to be **after** state and action declarations! */
let component = ReasonReact.reducerComponent("Kid");

/* greeting and children are props. `children` isn't used, therefore ignored.
   We ignore it by prepending it with an underscore */
let make = (~name, _children) => {
  /* spread the other default fields of component here and override a few */
  ...component,

  initialState: () => {present: true},

  /* State transitions */
  reducer: (action, state) =>
    switch (action) {
    | Toggle => ReasonReact.Update({...state, present: ! state.present})
    },

  render: self => {
  let style = self.state.present ? "green" : "red";
  let status = self.state.present ? "In" : "Out";
    <div>
      <button style=ReactDOMRe.Style.make(~color=style, ()) onClick=(_event => self.send(Toggle))>
        {ReasonReact.string(name ++ " - " ++ status)}
      </button>
    </div>;
  },
};
