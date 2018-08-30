/* FileConsole.re contains the buttons for roster preview and download */

open Types;

type action =
  | Click;

let component = ReasonReact.statelessComponent("FileConsole");

let make = (~school, ~extended_config, ~refreshClicked, ~resetClicked, _children) => {
  ...component,
  render: _self => {
    let dload =
      "data:application/octet-stream;charset=utf8;base64,"
      ++ btoa(Report.school(school, extended_config));
    <div>
      <p> {ReasonReact.string(Report.uncollected(school))} </p>
      <div className="preview_box">
        <ul> {Report.core_attendance_preview(school)} </ul>
        <ul> {Report.ext_attendance_preview(school, extended_config)} </ul>
      </div>
      <a href=dload> <button> {ReasonReact.string("Download")} </button> </a> <br /> <br />
      <button onClick=refreshClicked>{ReasonReact.string("Refresh connection")}</button>
      {ReasonReact.string(" ")}
      <button onClick=resetClicked>{ReasonReact.string("Clear Attendance")}</button>
    </div>;
  },
};
