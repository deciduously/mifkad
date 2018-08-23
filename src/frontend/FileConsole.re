/* FileConsole.re contains the buttons for roster preview and download */

open Types;

type action =
  | Click;

let component = ReasonReact.statelessComponent("FileConsole");

let make = (~school, ~dayChangeClick, ~extended_config, _children) => {
  ...component,
  render: _self => {
    let dload =
      "data:application/octet-stream;charset=utf8;base64,"
      ++ btoa(Report.school(school, extended_config));
    <div id="fileconsole">
      <button id="console" onClick=dayChangeClick>
        (ReasonReact.string("Pick a different day"))
      </button>
      <button
        onClick=(_event => alert(Report.school(school, extended_config)))>
        (ReasonReact.string("Preview"))
      </button>
      <a href=dload> <button> (ReasonReact.string("Download")) </button> </a>
    </div>;
  },
};
