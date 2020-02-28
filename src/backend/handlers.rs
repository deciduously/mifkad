// handlers.rs defines the actix_web handlers
use crate::data::{init_db, reset_db, reset_extday, write_db};
use crate::schema::{ExtendedDayConfig, School};
use crate::AppState;
use actix_web::{
    self, fs::NamedFile, AsyncResponder, HttpRequest, HttpResponse, Json, Path, State,
};
use futures::{future::result, Future};
use std::{
    clone::Clone,
    io::{prelude::Read, BufReader},
    path::PathBuf,
    str::FromStr,
};

// The types of actions adjust_school knows how to do
#[derive(Debug, Deserialize, Serialize)]
pub enum Action {
    Toggle,
    AddExt,
    Collect,
    Reset,
}

impl FromStr for Action {
    type Err = ::std::io::Error;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "toggle" | "Toggle" => Ok(Action::Toggle),
            "addext" | "AddExt" => Ok(Action::AddExt),
            "collect" | "Collect" => Ok(Action::Collect),
            "reset" | "Reset" => Ok(Action::Reset),
            _ => Err(::std::io::Error::new(
                ::std::io::ErrorKind::InvalidInput,
                "Not an Action",
            )),
        }
    }
}

pub fn index(
    _req: &HttpRequest<AppState>,
) -> Box<dyn Future<Item = HttpResponse, Error = actix_web::Error>> {
    let path: PathBuf = PathBuf::from("./mifkad-assets/index.html");

    let f = NamedFile::open(&path)
        .unwrap_or_else(|_| panic!("Could not open {}", path.to_str().unwrap()));
    let mut bfr = BufReader::new(f.file());
    let mut ret = String::new();
    bfr.read_to_string(&mut ret)
        .unwrap_or_else(|_| panic!("could not read index file"));

    result(Ok(HttpResponse::Ok().content_type("text/html").body(ret))).responder()
}

// Replace the extended config with the incoming POST data
pub fn new_extended_config(
    (newconf, state): (Json<ExtendedDayConfig>, State<AppState>),
) -> Box<dyn Future<Item = HttpResponse, Error = actix_web::Error>> {
    warn!("Resetting extended day config!");
    reset_extday(&newconf.clone(), &state.config).unwrap();
    let mut a = state.school.write().unwrap();
    // update app state
    (*a).replace_extended_config(&newconf.into_inner());
    // sync app state to disk
    write_db(&*a).unwrap();
    result(Ok(HttpResponse::Ok().finish())).responder()
}

// The RwLock read handler
pub fn school_today(
    req: &HttpRequest<AppState>,
) -> Box<dyn Future<Item = Json<School>, Error = actix_web::Error>> {
    // Grab a non-blocking read lock and return the result as Json
    let a = req.state().school.read().unwrap();
    let ret = Json((*a).clone());
    debug!("school today: {}", json!(*a));
    result(Ok(ret)).responder()
}

// the RwLock write handler
pub fn adjust_school(
    (path, state): (Path<(String, u32)>, State<AppState>),
) -> Box<dyn Future<Item = Json<School>, Error = actix_web::Error>> {
    use self::Action::*;
    let action = Action::from_str(&path.0).unwrap();
    let id = path.1;

    {
        // Grab a blocking write lock inside inner scope
        let mut a = state.school.write().unwrap();

        // Perform the mutation
        match action {
            Toggle => (*a).toggle_kid(id),
            AddExt => (*a).addext_kid(id),
            Collect => (*a).collect_room(id),
            Reset => {
                reset_db(&state.config).unwrap();
                (*a) = init_db(&state.config).unwrap();
            }
        }
        // blocking lock is dropped here
    }

    // grab a new non-blocking reader
    let a = state.school.read().unwrap();

    // Sync the on-disk DB and return the json
    write_db(&*a).unwrap();
    result(Ok(Json((*a).clone()))).responder()
}
