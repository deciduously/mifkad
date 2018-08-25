# mifkad

[![Build Status](https://travis-ci.org/deciduously/mifkad.svg?branch=master)](https://travis-ci.org/deciduously/mifkad)

Attendance taking web app.

This is a ReasonReact/actix_web rewrite of my prior [attendance](https://github.com/deciduously/attendance) Reagent/Rocket project, which can be considered deprecated.  There's still a lot of Clojure love, but pragmatism and all...

## Dependencies

### Release

* Windows.  Targeting 7, probably works on earlier versions?  Don't know, or particularly need to know.

### Development

* Stable rust 1.26.0+ (uses `impl Trait`)
* npm/yarn

Yarn will fetch and build `bs-platform` for you, which does include compiling the OCaml compiler.  So the first run of `yarn install` is a bit bulky on this one unless you're already useing ReasonML.  This build setup has been tested as-is on Linux (4.18+, but again, probably fine on almost anything) and Windows x86_64 platforms.  In Windows, you do need to use WSL to build the frontend, which is Windows 10 exclusive.  I run the whole dev environemnt in WSL, and then build the backend using the native `x86_64-pc-windows-msvc` backend inside the `VS2015 x64 Native Tools Command Prompt` with `yarn build:rs`.

## Usage

Download a [release](https://github.com/deciduously/mifkad/releases).  I am currently only providing Windows builds as that's probably the only environment this will run on.  Extract the zip file to a folder of your choice and ensure data is available at `.\current.xls`.  In the target environment this is already taken care of.  Then double click `mifkad.exe` and point your browser to the URL given, currently always `127.0.0.1:8080`.  Have a blast clicking buttons for hours on end.

## Develop

 The build is controlled via `npm` scripts defined in `package.json`.  The following tasks are useful:

* `yarn start` - Run the Reason->JS->webnpack dev pipeline and `cargo watch -x test -x run` and watch for changes
* `yarn prod` - Run all production builds once, first the apps in parallel and then webpack
* `yarn prod:win` - Just run the Reason pipeline, so you can run Rust natively with `yarn build:rs`


Then open a browser to `localhost:8080`, pick a day, and log attendance until the cows home.

It expects an input file under `./current.xls`. There's a sample under `sample/` with fudged data.  It intentionally has mismatched headers and things - that's how they come outta the big box, except for all the extraneous info I've removed.   Redacted, if I want to sound fancy.  And I do.  Rows or data in rows that aren't what I care about don't matter.

## Acknowledgements

`wac.exe` is from [here](https://github.com/aslakhellesoy/wac), because the target environment is Windows 7 and I don't wanna give up my ANSI-colored logging crate.  It just makes em work instead of printiing gibberish.