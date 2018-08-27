-- Your SQL goes here
CREATE TABLE records (
  id INTEGER PRIMARY KEY,
  name TEXT NOT NULL,
  date TEXT NOT NULL,
  classroom TEXT NOT NULL,
  expected TEXT NOT NULL,
  actual BOOLEAN NOT NULL,
)
