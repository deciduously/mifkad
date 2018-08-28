# mifkad

[![Build Status](https://travis-ci.org/deciduously/mifkad.svg?branch=master)](https://travis-ci.org/deciduously/mifkad)

Attendance taking web app.

![demo data](https://i.imgur.com/kS8NW4o.png)

This is a ReasonReact/actix_web rewrite of my prior [attendance](https://github.com/deciduously/attendance) Reagent/Rocket project, which can be considered deprecated.  There's still a lot of Clojure love, but pragmatism and all...

This is not an app I expect will be useful to anyone but myself - the only reason I include such a thorough README is because it will likely need maintence about once a year, and I don't have nearly that attention span.

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

Depending on your hardware, the backend may take quite some time to build - `actix_web` brings along with it a somewhat large set of crates that all need to be rebuilt after invoking `cargo clean`.  It's usually just shy of 3 minutes on my i7-8700 desktop, and closer to 10 on my old AMD APU A10-5750M laptop.  Personally, I've never been too bothered by hefty build times if the resulting executable is reasonably sized and highly performant, and the dev cycle is reasonable even on the laptop, as it doesn't clean everything and doesn't use optimizations.  `mifkad.exe` clocks in perhaps a tad larger than I'd like for the functionality but in the grand scheme of things, I'm not losing sleep over a 5MB executable that isn't getting transferred around a lot.  It starts up practically instantaneously - at least faster than any other application we use - even on the old hardware at work and barely sips on system resources, so I'm happy.

It expects an input file under `.\current.xls`. There's a sample at `sample\test.xls` with fudged data.  It intentionally has mismatched headers and things - that's how they come outta the big box, except for all the extraneous info I've removed - redacted, if I want to sound fancy.  And I do.  Rows or data in rows that aren't what I care about don't matter.
