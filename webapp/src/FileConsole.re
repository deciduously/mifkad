/* FileConsole.re contains the buttons for roster uploading */

open Types;

type action =
  | Click /* this needs to go after state and action */;

let component = ReasonReact.statelessComponent("FileConsole");

let make = (~school, ~dayChangeClick, _children) => {
  ...component,
  render: _self => {
    let dload =
      "data:application/octet-stream;charset=utf8;base64,"
      ++ btoa(Report.school(school));
    <div id="fileconsole">
      <button id="console" onClick=dayChangeClick>
        {ReasonReact.string("Pick a different day")}
      </button>
      <button onClick={_event => alert(Report.school(school))}>
        {ReasonReact.string("Preview")}
      </button>
      <a href=dload> <button> {ReasonReact.string("Download")} </button> </a>
    </div>;
  },
};
