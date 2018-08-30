# mifkad

[![Build Status](https://travis-ci.org/deciduously/mifkad.svg?branch=master)](https://travis-ci.org/deciduously/mifkad)

Full-stack webapp for logging daily classroom attendance form an auto-generated enrollment report.  It stores state to disk on each click, so the current status is not affected by page reloads or app restarts except for the first run of each day.  You could have multiple connections open at once, but it will only notice changes other clients make if you change something or if you refresh....for now (maybe).

![demo data](https://i.imgur.com/kS8NW4o.png)

This is rewrite of my prior [attendance](https://github.com/deciduously/attendance) ClojureScript project, which should be considered deprecated.

The backend is built using the [`actix_web`](https://actix.rs) framework for [Rust](https://www.rust-lang.org/), and the frontend is buiilt with [ReasonReact](https://reasonml.github.io/reason-react/).

## Dependencies

### Release

* Windows 7+.

It probably works on earlier versions.  I don't know but the mystery keeps life spicy, and this won't ever run on anything other than windows 7 and up barring some sorta cyber-temporal apocalypse.

### Development

* Stable rust 1.26.0+ (uses `impl Trait`)
* `cargo-watch` - run `cargo install cargo-watch` to obtain if needed.
* npm/yarn

Yarn will fetch and build `bs-platform` for you, which does include compiling the OCaml compiler.  So the first run of `yarn install` is a bit bulky on this one unless you're already using ReasonML.  This build setup has been tested as-is on Linux (4.18+, but again, probably fine on almost anything) and Windows x86_64 platforms with the `x86_64-pc-windows-msvc` toolchain.  On Windows, you may need to +use the `VS2015 x64 Native Tools Command Prompt`, which I know you have if you've managed to install `rust`.  `ring` v0.12 was not building outside of it, though now that dependency has been updated and I am able to build the app in a regular prompt.  I also find it easier to use the WSL for development and the frontend toolchain.

## Usage

Download a [release](https://github.com/deciduously/mifkad/releases).  I am currently only providing Windows builds as that's probably the only environment this will run on.  Extract the zip file to a folder of your choice and replace the included demo `.\current.xls` with real data.  In the target environment this is already taken care of.  Then double click `mifkad.exe` and point your browser to the URL given, currently always `127.0.0.1:8080`.  Have a blast clicking buttons for hours on end.  On Windows, `mifkad-launcher.bat` is provided as a one-click solution for starting the server and opening the webpage in Google Chrome.  Leave the launcher in the same directory as the executable, and create a shortcut to it anywhere you like.

## Develop

```sh
git clone https://github.com/deciduously/mifkad
cd mifkad
yarn install
yarn start
```

 The build is controlled via `npm` scripts defined in `package.json`.  The following meta-tasks are defined:

* `yarn start` - Run the Reason->JS->webpack dev pipeline and `cargo watch -x test -x run` and watch for changes, serving the app on `127.0.0.1:8080`
* `yarn prod` - Run all production builds once, first the apps in parallel and then webpack - this may take a while
* `yarn clean` - run `bsb -clean-world` and `cargo clean` - `yarn prod` will invoke this first.

I use `release-win.bat` to automate the release.  On Windows, I run `yarn prod:win` in WSL, then `yarn build:rs` natively, and then invoke the batchfile.

Depending on your hardware, the backend may take quite some time to build in poroduction mode - `actix_web` brings along with it a somewhat large set of crates that all need to be rebuilt after invoking `cargo clean`.  The dev build is more reasonable.

It expects an input file under `.\current.xls`. There's a sample at `sample\test.xls` with fudged data.  It intentionally has mismatched headers and things - that's how they come outta the big box, except for all the extraneous info I've removed - redacted, if I want to sound fancy.  And I do.  Rows or data in rows that aren't what I care about don't matter.
