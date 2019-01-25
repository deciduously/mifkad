# mifkad

[![Build Status](https://travis-ci.org/deciduously/mifkad.svg?branch=master)](https://travis-ci.org/deciduously/mifkad)

Full-stack webapp for logging daily classroom attendance using an auto-generated enrollment Excel sheet as input.  It stores state to disk on each click, so the current status is not affected by page reloads or app restarts except for the first run of each day.

![demo data](https://i.imgur.com/zgwpnkQ.png)

This is rewrite of my prior [attendance](https://github.com/deciduously/attendance) ClojureScript project, which should be considered deprecated.

The backend is built using the [`actix_web`](https://actix.rs) framework for [Rust](https://www.rust-lang.org/), and the frontend is built with [ReasonReact](https://reasonml.github.io/reason-react/).

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

Accepts an optional configuration file at `Mifkad.toml` in the same directory as the executable (until I make a flag for it) like the following.  If absent the default values listed are used.  

The full distribution consists of the following components:

1. Mifkad.exe – this is a file scraper and webserver.  When executed it loads the attendance data and listens for requests at “localhost:8080”. On launch, it will first ensure an extended day configuration exists and if not create one.  Then it will look in “mifkad-assets/db” for a json file corresponding to today’s date, e.g. “20181107.json”.  These files are raw JSON representations of the app’s internal data structure.  If no “db” folder exists, it will create one.  If it finds a file for the current date it will load the data and store a file pointer to this file for updating state.  Below is the expected output if this file exists:![normal run](https://i.imgur.com/KRU2yRX.png)If no file exists, it will first create one.  To do so, it looks for a file named “current.xls” in the same directory as the executable itself.  If it finds one it will read the sheet line by line, detecting classroom declarations and child declarations.  When complete it will write the scraped enrollment out to a json file, and re-start itself from the beginning.  This time it will find the json file it just created and continue the boot process.  Below is a screenshot of the expected output from the first run of the day: ![first run](https://i.imgur.com/90DIdfm.png)Once satisfied it has valid backing data, it will attempt to assume control of port 8080.  If successful it will spawn a worker thread for each logical core on the machine to asynchronously process HTTP requests.  If invoked from the provided launcher script, Google Chrome will have opened and navigated to “localhost:8080”.  This request will be noted in the event log as first a GET to “/”, which will fetch the frontend followed by a GET to “/school/today” to query the current state:![opening requests](https://i.imgur.com/f27JTOJ.png)

2. Mifkad-launcher.bat – this is a small batch file which simply executes the above binary and opens up Google Chrome to the correct page, “localhost:8080”.

3. Mifkad.toml – this is the configuration file.   TOML is a small language similar to INI, ideal for representing hash tables.  The verbosity field must be one of “Warn”, “Info”, “Debug”, or “Trace”, and the “Info” level should be sufficient for diagnosing runtime errors.  Here is the contents, given with their defaults.  Verbosity should be one of "Warn", "Info", "Debug", or "Trace".

```toml
port = 8080
roster = "current.xls"
extended_config = "extended_day.json"
verbosity = "Warn"
```

4. Extended_config.json – this file contains the current extended day configuration.  Its location is set in Mifkad.toml.  If not present, mifkad.exe will create a new one using a hardcoded default before doing anything else.  I can be edited by hand if needed but I recommend using the UI to manipulate it instead, to ensure it remains properly formatted.

5.  Mifkad-assets – this directory contains the static assets necessary to render the page and the “db” folder containing the app’s data.  It must be present and populated for the app to run.  Inside there are the following components:
    1. Index.html – this is the webpage that serves as an entry point for the script, served by the GET / endpoint.  It only contains an empty “div” element and points to the JS, CSS, and favicon  – the app is fully defined in the JavaScript file.
    2. “js\” contains a single file, index.js.  This JavaScript is output by a compiler from a different language and minified – it’s technically editable by hand but I don’t recommend it.  It contains all frontend logic as well as the open-source React.js library from Facebook.  I do not distribute the source code with the bundle but it can be found on GitHub if necessary.  Fresh zipped bundles of the latest build can be downloaded from that link as well if any or all of the bundle gets lost or corrupted.
    3. “css\ contains a single file, index.css – this is the stylesheet referenced in index.html.
    4. “db\” – this folder is managed by the application.  It and its contents can be safely deleted, the app will regenerate it as needed.
    5. “favicon.ico” is a small icon of a clipboard to display in the browser bar.

The app’s output is displayed at the top and each classroom is rendered as a list of buttons.  Click the kid’s name to toggle their attendance status, and click the “Pink Sheet” button to add them to extended day on an ad-hoc basis.  The “Need to Finish” buttons toggle to “Ready” and only affect the list at the top of the page – they’re only there as a reminder for the user during collection.  The app is automatically including any child with an end time past 4pm on the Site:Enrollment input file.
On each click the app makes a GET request to the backend.  This output is suppressed in normal usage, use the “INFO” verbosity setting to see each request:![normal requests](https://i.imgur.com/FmrdhgA.png)

This request will consist of a verb: “toggle”, “addext”, or “collect”, followed by an ID.  Each kid and classroom is assigned a unique ID when the enrollment is scraped.  This request will cause the change to be recorded in today’s JSON file.  The frontend displays the toggled state immediately, but will automatically toggle back if the backend responds with anything other than “200 - Ok”, which will indicate to the user the server process has crashed.
These requests should not collide – incoming requests are handled asynchronously by n * cores worker threads and the app state is only accessed via a mutex which allows either one single writer or multiple concurrent readers at a time but not both.  Multiple simultaneous incoming requests are queued and processed in FIFO order.  If another instance of mifkad currently holds a lock, all requests in the current instance will be blocked until the lock is released.  In actual usage, there will rarely be more than one user at a time.  One missing feature is periodic update of the state – the user at present needs to click “Refresh Connection” to become aware of any changes made from other sessions.
Once attendance is processed, the user can opt to click the “Download” button, which will prompt the user to choose a location and name to save a copy of the current output as a text file on disk, or they can simply highlight the top section and copy/paste it into an email.
If the “mifkad.exe” process is terminated, the app will display the following message after attempting to register a click:![error](https://i.imgur.com/r5AYK5M.png)

In this case, ensure “mifkad.exe” is running and use the “try to reconnect” button to force it to attempt to reconnect to the server (by issuing GET /school/today).  Because the data is stored to disk each click, restarting the binary and clicking this button should succeed with no data loss since the last successful click.

The extended day configuration is calculated at runtime (and on each click), and can be reconfigured using the UI found at the bottom of the page:![extended config](https://i.imgur.com/IMQ35mc.png)

There is a list of all available extended day rooms, each with a button labelled Remove, and an area to create a new extended day room. The first text box is the letter assignment and must be of format “XE”, a letter followed by “E”. It will yell at you if you try to submit something that doesn’t match the prescribed format or a label that already exists. The second is for the maximum capacity to display – it still doesn’t do any sort of verification that you’re within the limit, it’s just there as a visual reminder. Hitting the “Enter” key with your cursor in either of these two boxes will create the new entry and you’ll see it appear in the list, though you won’t see it in the roster above until you assign a classroom to it.

One limitation is that you currently cannot adjust a current extended day room’s capacity using this interface. As a workaround, you can Remove it and then create the same letter again with the new desired capacity, and re-assign the proper rooms that got unassigned when you deleted it. Alternatively you can edit the file extended_config.json by hand, but this workaround is probably easier. I feel I should mention that there’s a small bug somewhere – sometimes rooms will pick up one of the core room’s capacities instead of what you specifically assigned. Sometimes adding another room to it will fix it, sometimes not. I haven’t figured out what the heck’s going on there yet, as far as I can tell there’s no reason that should be happening at all, but it seems pretty rare for now and as soon as I figure it out I’ll make it stop!

Next you’ll see a list of each core room with a drop-down box showing the current extended day room it’s funneling to. The drop-down contains each available room (including any you’ve created) or “Unassigned”, which is used when you Remove a room that’s currently in use. This makes it pretty easy to swap one out – for example, to change the capacity in room AE, you’d click Remove and then make a new AE. When you removed the original AE, any room that had it assigned will flip to “Unassigned”, so it’s easy to see which ones need to be added back. Any unassigned kids will make their own quasi-category called “Room Unassigned” and show up in your email as “Room U”, so make sure to assign each class before sending it out!

Any changes you make to the setup are temporary. You’ll see the results immediately on the page and in your report, but you can always reset to what you had by clicking “Refresh Connection”, reloading the webpage, or restarting “mifkad.exe” until you click Save Config. After any of these methods it will fall back to whatever is set in “extended_config.json” (name/location are configurable, details below). If this file doesn’t exist for some reason it will be created using a hardcoded default when you run mifkad.exe. To save a new configuration when you’re done tweaking it, click “Save Config” but be careful – this will immediately replace this file with the new setup, with no way to undo it. The app will ask you to confirm once before making any changes. If you click “Cancel” your changes won’t go away so you can keep tweaking it but they have not been saved – reloading the page or using “Refresh Connection” will reset to whatever you have saved. If you click “yes” subsequent restarts of the app will use your new setup instead from now on. To get back to the default you could simply delete this file and restart mifkad - it will regenerate with the hardcoded setup, losing any changes you’ve made. As mentioned above, you’re also welcome to edit this file by hand. You’ll then need to re-start mifkad.exe to grab any edits made.

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

Depending on your hardware, the backend may take quite some time to build in production mode - `actix_web` brings along with it a somewhat large set of crates that all need to be rebuilt after invoking `cargo clean`.  The dev build is more reasonable.

It expects an input file under `.\current.xls`. There's a sample at `sample\test.xls` with fudged data.  It intentionally has mismatched headers and things - that's how they come outta the big box, except for all the extraneous info I've removed - redacted, if I want to sound fancy.  And I do.  Rows or data in rows that aren't what I care about don't matter.
