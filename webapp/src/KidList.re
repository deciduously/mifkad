/* Kidlist.re is responsible for rendering the list of kids in a room */
open Types;

let component = ReasonReact.statelessComponent("KidList");

let make = (~kids: array(kid), ~onClick, _children) => {
  ...component,
  render: _self =>
    <ul className="kidlist">
      (
        Array.map(
          k => <li key=k.name className="kid"> <Kid kid=k onClick /> </li>,
          kids,
        )
        |> ReasonReact.array
      )
    </ul>,
};