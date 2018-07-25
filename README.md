# mifkad

Attendance taking web app.

This is a ReasonReact/actix_web/cap'nproto rewrite of my prior [attendance](https://github.com/deciduously/attendance) Reagent/Rocket project.  As much as I love working with Clojure, I was spending more time with the tooling I felt was reasonable while porting that to re-frame and adding functionality.  Same goes for Rocket - great to work with, but nightly rust was a hassle.  It turns out actix is great, anyhow.

So, of course, I hopped ship to something else new and niche.  Let's see how this goes.

## Usage
 The build is contralled via `npm` scripts defined in `package.json`.  I prefer `yarn`, use what makes you happy.  `start` runs the Reason filewatcher, and `webpack` runs webpack.  Use `build` and `webpack:production` for production.  I haven't unified the build tooling yet, so you'll also need to spin up the server with something like `cargo watch -x run`.
