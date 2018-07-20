/* App.re - main container */
let component = ReasonReact.statelessComponent("App");

/* underscores indicate unused, like in Rust */
/* last one must be children */
let make = (_children) => {
  ...component, /* spread the template's other defaults into here - otherwise it'd be in a double [|[|<child>|]|] */
  render: (_self) => {
  <div id="app">
    <h1>{ReasonReact.string("Attendance")}</h1>
    <hr />
    <FileConsole />
    <hr />
    <Roster roster="RosterPlaceholder" />
    <hr />
    <Roster roster="RosterPlaceholder2" />
    <hr />
    <footer>
      {ReasonReact.string(". \xA9 2018 deciduously -")} <a href="https://github.com/deciduously/mifkad">{ReasonReact.string("source")}</a>
    </footer>
    </div>
  }
};
