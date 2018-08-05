/* FileConsole.re contains the buttons for roster uploading */

type action =
  | Click /* this needs to go after state and action */;

let component = ReasonReact.statelessComponent("FileConsole");

let make = (~onClick, _children) => {
  ...component,
  render: self =>
    <div id="fileconsole">
      <button id="console" onClick>
        (ReasonReact.string("Pick a different day"))
      </button>
      <FileUpload id="roster" filetype="csv" />
    </div>,
};