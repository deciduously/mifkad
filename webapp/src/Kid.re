/* State declaration */
type state = {
  color: bool,
  show: bool,
};

/* Action declaration */
type action =
  | Click
  | Toggle;

/* Component template declaration.
   Needs to be **after** state and action declarations! */
let component = ReasonReact.reducerComponent("Kid");

/* greeting and children are props. `children` isn't used, therefore ignored.
   We ignore it by prepending it with an underscore */
let make = (~greeting, _children) => {
  /* spread the other default fields of component here and override a few */
  ...component,

  initialState: () => {color: false, show: true},

  /* State transitions */
  reducer: (action, state) =>
    switch (action) {
    | Click => ReasonReact.Update({...state, color: ! state.color})
    | Toggle => ReasonReact.Update({...state, show: ! state.show})
    },

  render: self => {
  let message = "I am but a child";
  let style = self.state.color ? "red" : "blue";
    <div>
      <button style=ReactDOMRe.Style.make(~color=style, ()) onClick=(_event => self.send(Click))>
        (ReasonReact.string(message))
      </button>
      <button onClick=(_event => self.send(Toggle))>
        (ReasonReact.string("Toggle greeting"))
      </button>
      (self.state.show ? ReasonReact.string(greeting) : ReasonReact.null)
    </div>;
  },
};
