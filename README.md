# mifkad

[![Build Status](https://travis-ci.org/deciduously/mifkad.svg?branch=master)](https://travis-ci.org/deciduously/mifkad)

Attendance taking web app.

This is a ReasonReact/actix_web rewrite of my prior [attendance](https://github.com/deciduously/attendance) Reagent/Rocket project, which can be considered deprecated.  There's still a lot of Clojure love, but pragmatism and all...

## Dependencies

### Release

* Windows 7+.

It probably works on earlier versions.  I don't know but the mystery keeps life spicy, and this won't ever run on anything other than windows 7 and up barring some sorta cyber-temporal apocalypse.

### Development

* Stable rust 1.26.0+ (uses `impl Trait`)
* `cargo-watch` - run `cargo install cargo-watch` to obtain if needed.
* npm/yarn

Yarn will fetch and build `bs-platform` for you, which does include compiling the OCaml compiler.  So the first run of `yarn install` is a bit bulky on this one unless you're already useing ReasonML.  This build setup has been tested as-is on Linux (4.18+, but again, probably fine on almost anything) and Windows x86_64 platforms.  In Windows, you do need to use WSL to build the frontend, which is Windows 10 exclusive.  I run the whole dev environemnt in WSL, and then build the backend using the native `x86_64-pc-windows-msvc` backend inside the `VS2015 x64 Native Tools Command Prompt` with `yarn build:rs`.

## Usage

Download a [release](https://github.com/deciduously/mifkad/releases).  I am currently only providing Windows builds as that's probably the only environment this will run on.  Extract the zip file to a folder of your choice and ensure data is available at `.\current.xls`.  In the target environment this is already taken care of.  Then double click `mifkad.exe` and point your browser to the URL given, currently always `127.0.0.1:8080`.  Have a blast clicking buttons for hours on end.

## Develop

```sh
git clone https://github.com/deciduously/mifkad
cd mifkad
yarn install
yarn start
```

 The build is controlled via `npm` scripts defined in `package.json`.  The following meta-tasks are defined:

* `yarn start` - Run the Reason->JS->webnpack dev pipeline and `cargo watch -x test -x run` and watch for changes, serving the app on `127.0.0.1:8080`
* `yarn prod` - Run all production builds once, first the apps in parallel and then webpack
* `yarn prod:win` - Just run the Reason pipeline, so you can run Rust natively with `yarn build:rs`
* `yarn clean` - run `bsb -clean-world` and `cargo clean`

It expects an input file under `.\current.xls`. There's a sample at `sample\test.xls` with fudged data.  It intentionally has mismatched headers and things - that's how they come outta the big box, except for all the extraneous info I've removed - redacted, if I want to sound fancy.  And I do.  Rows or data in rows that aren't what I care about don't matter.

## Acknowledgements

`wac.exe` is from [aslahellesoy](https://github.com/aslakhellesoy/wac), used because the target environment is Windows 7 and I don't wanna give up my ANSI-colored logging crate.  It just makes the codes work instead of printing obfuscating gibberish.