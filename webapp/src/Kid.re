/* Kid.re renders a single kid */

open Types /* Component template declaration.
   Needs to be **after** state and action declarations!
   This is a stateless component - all it will do it pass the message back up to the App root */;

let component = ReasonReact.statelessComponent("Kid") /* greeting and children are props. `children` isn't used, therefore ignored.
   We ignore it by prepending it with an underscore */;

let make = (~kid: kid, ~onClick, ~core, _children) => {
  let click = _event => onClick(kid) /* When the button is clicked, pass the kid on up with the callback */;
  {
    /* spread the other default fields of component here and override a few */
    ...component,
    render: _self => {
      let button_class = kid.schedule.actual ? "In" : "Out";
      
      let to_disp_name = name => {
        /* Takes FIRSTNAME LASTNAME and returns Firstname Lastname */
        let idx_of_spc = String.index(name, ' ');
        
        let first_name =
          String.sub(name, 0, idx_of_spc) /* start_idx, len */
        |> String.lowercase
        |> String.capitalize;
        
        let start_of_last = idx_of_spc + 1;
        
        let last_name = String.sub(name, start_of_last, String.length(name) - start_of_last)
        |> String.lowercase
        |> String.capitalize;
        
        first_name ++ " " ++ last_name;
      };
      
      core ?
      <button className=button_class onClick=click>
        (
          ReasonReact.string(
            to_disp_name(kid.name) ++ " - " ++ button_class,
          )
        )
    </button>
    : <div className=button_class>(ReasonReact.string(to_disp_name(kid.name) ++ " - " ++ button_class))</div>;
    },
  };
};
