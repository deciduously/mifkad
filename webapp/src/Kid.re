/* Kid.re renders a single kid */

open Types;

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
let make = (~kid : kid, _children) => {
  /* spread the other default fields of component here and override a few */
  ...component,

  initialState: () => {present: true},

  /* State transitions */
  reducer: (action, state) =>
    switch (action) {
    | Toggle => ReasonReact.Update({present: ! state.present}) /* present is the whole state, no ...state needed */
    },

  render: self => {
  let button_class = self.state.present ? "In" : "Out";
      <button className=button_class onClick=(_event => self.send(Toggle))>
        {ReasonReact.string(kid.name ++ " - " ++ kid.schedule.expected ++ " : " ++ button_class)}
      </button>
  },
};
