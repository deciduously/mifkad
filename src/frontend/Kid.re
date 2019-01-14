/* Kid.re renders a single kid */

open Types;

type category =
  /* A Kid is either in the Core or Extended roster, and if in core, expected in extended or not*/
  | NoButton
  | ButtonCore
  | ButtonExtended;

let component = ReasonReact.statelessComponent("Kid");

let make = (~kid: kid, ~kidClicked, ~addextClicked, ~core, _children) => {
  let toggleclick = _event => kidClicked(kid) /* When the button is clicked, pass the kid on up with the callback */;
  let addextclick = _event => addextClicked(kid);
  {
    ...component,
    render: _self => {
      let button_class = kid.schedule.actual ? "In" : "Out";
      let addext_class =
        kid.schedule.expected == "Added" ? "pink_sheet" : "no_pink_sheet";

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
          NoButton /* Ben - nested ternaries are hard to read */;

      switch (category(kid)) {
      | NoButton =>
        <div className=button_class>
          {ReasonReact.string(to_disp_name(kid.name))}
        </div>
      | ButtonExtended =>
        <button className=button_class onClick=toggleclick>
          {ReasonReact.string(to_disp_name(kid.name))}
        </button>
      | ButtonCore =>
        <div>
          <button className=button_class onClick=toggleclick>
            {ReasonReact.string(to_disp_name(kid.name))}
          </button>
          <button className=addext_class onClick=addextclick>
            {
              ReasonReact.string(
                (kid.schedule.expected == "Added" ? "Undo p" : "P")
                ++ "ink sheet",
              )
            }
          </button>
        </div>
      };
    },
  };
};
