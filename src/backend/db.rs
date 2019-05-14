use crate::schema::Kid;

use actix_web::{web, Error as AWError};
use failure::Error;
use futures::Future;
use r2d2;
use r2d2_sqlite;
use rusqlite::NO_PARAMS;

pub type Pool = r2d2::Pool<r2d2_sqlite::SqliteConnectionManager>;
pub type Connection = r2d2::PooledConnection<r2d2_sqlite::SqliteConnectionManager>;

pub enum Query {
    GetChild(String)
}

pub fn execute(pool: &Pool, query: Query) -> impl Future<Item = Vec<Kid>, Error = AWError> {
    let pool = pool.clone();
    web::block(move || match query {
        Query::GetChild(name) => get_child(&name, pool.get()?),
    })
    .from_err()
}

fn get_child(name: &str, conn: Connection) -> Result<Vec<Kid>, Error> {
    unimplemented!()
}