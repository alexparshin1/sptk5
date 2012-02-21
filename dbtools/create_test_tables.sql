-- This SQL script creates the test tables.
-- It should be executed only once, before the attempt to test sql2cpp.

--drop table test_book;
--drop table test_author;

CREATE TABLE test_author (
  author_id SERIAL PRIMARY KEY,
  author_name VARCHAR(30)
);

CREATE TABLE test_book (
  book_id SERIAL PRIMARY KEY,
  book_title VARCHAR(80),
  book_published TIMESTAMP,
  author_id INT REFERENCES test_author(author_id)
);

