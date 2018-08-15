/* Kid.re renders a single kid */

open Types;

type category =
  /* A Kid is either in the Core or Extended roster, and if in core, expected in extended or not*/
  | NoButton
  | ButtonCore
  | ButtonExtended;

let component = ReasonReact.statelessComponent("Kid");

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
          String.sub(name, 0, idx_of_spc)  /* start_idx, len */
          |> String.lowercase
          |> String.capitalize;

        let start_of_last = idx_of_spc + 1;

        let last_name =
          String.sub(
            name,
            start_of_last,
            String.length(name) - start_of_last,
          )
          |> String.lowercase
          |> String.capitalize;

        first_name ++ " " ++ last_name ++ " - " ++ button_class;
      };

      let category = kid =>
        core ?
          kid.schedule.expected == "Extended" ? ButtonExtended : ButtonCore :
          NoButton;

      switch (category(kid)) {
      | NoButton =>
        <div className=button_class>
          {ReasonReact.string(to_disp_name(kid.name))}
        </div>
      | ButtonExtended =>
        <button className=button_class onClick=click>
          {ReasonReact.string(to_disp_name(kid.name))}
        </button>
      | ButtonCore =>
        <div>
          <button className=button_class onClick=click>
            {ReasonReact.string(to_disp_name(kid.name))}
          </button>
          <input type_="checkbox" name="addext" />
        </div>
      };
    },
  };
};
