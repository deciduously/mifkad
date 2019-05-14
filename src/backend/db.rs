use crate::schema::Kid;

use actix_web::error;
use futures::Future;
use r2d2;
use r2d2_sqlite;
use rusqlite::NO_PARAMS;
use std::error::Error;

pub type Pool = r2d2::Pool<r2d2_sqlite::SqliteConnectionManager>;
pub type Connection = r2d2::PooledConnection<r2d2_sqlite::SqliteConnectionManager>;

pub enum Query {
    GetChild(String)
}

pub fn execute(pool: &Pool, query: Query) -> impl Future<Item = Vec<Kid>, Error = error::Error> {
    let pool = pool.clone();
    unimplemented!()
}

