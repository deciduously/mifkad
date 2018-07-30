/* Kid.re renders a single kid */

open Types;

/* Component template declaration.
   Needs to be **after** state and action declarations! 
   This is a stateless component - all it will do it pass the message back up to the App root */
let component = ReasonReact.statelessComponent("Kid");

/* greeting and children are props. `children` isn't used, therefore ignored.
   We ignore it by prepending it with an underscore */
let make = (~kid : kid, ~onClick, _children) => {
  let click = (_event) => onClick(kid); /* When the button is clicked, pass the kid on up with the callback */
  {
  /* spread the other default fields of component here and override a few */
    ...component,
    
    render: _self => {
      let button_class = kid.schedule.actual ? "In" : "Out";
      <button className=button_class onClick=click>
          {ReasonReact.string(kid.name ++ " - " ++ kid.schedule.expected ++ " : " ++ button_class)}
    </button>
    },
  }
};
