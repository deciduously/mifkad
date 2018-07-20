/* FileUpload.re is a small widget for rendering a file upload button */
let component = ReasonReact.statelessComponent("FileUpload");

let make = (~id, ~filetype, _children) => {
  ...component, 
  render: (self) => {
      <input id=id _type="file" accept=filetype onChange=((_event => self.send(Js.log("FileUpload click")))) />
    }
}
