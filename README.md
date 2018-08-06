# mifkad

[![Build Status](https://travis-ci.org/deciduously/mifkad.svg?branch=master)](https://travis-ci.org/deciduously/mifkad)

Attendance taking web app.

This is a ReasonReact/actix_web/cap'nproto rewrite of my prior [attendance](https://github.com/deciduously/attendance) Reagent/Rocket project.  As much as I love working with Clojure, I was spending more time with the tooling I felt was reasonable while porting that to re-frame and adding functionality.  Same goes for Rocket - great to work with, but nightly rust was a hassle.  It turns out actix is great, anyhow.

So, of course, I hopped ship to something else new and niche.  Let's see how this goes.

## Usage
 The build is contralled via `npm` scripts defined in `package.json`.  I prefer `yarn`, use what makes you happy.
 
 I haven't done build tooling yet, so for now, open 3 terminals (or tmux or something) and run each of these in their own:

* `yarn start` - compile Reason files to JS and watch for changes
* `yarn webpack` - Bundle the JS for serving and watch for changes
* `cargo watch -x run` - Build and run the server, watching for changes.

Then open a browser to `localhost:8080`, pick a day, and log attendance until the cows home.

It expects an input file under `sample/sample_enroll_all_detail_week.xlsx`.  I have not included the sample input in the repo.  Someday I'll type up the expected format but as its an auto-generated internal report iwthin our organization and this is an internal tool, no users of this software will ever need to know the exact format.  We just request it from the database and pop it in :).  If you're *super* bored, the parsing logic lives in [`src/data.rs`](https://github.com/deciduously/mifkad/blob/master/src/data.rs) and you could totes reverse engineer it.  But, like, get a better hobby.  Yikes.

Obviously, this is a temporary requirement - this file location will be submitted by either command line flag, env variable, or via the web frontend.
