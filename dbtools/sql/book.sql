-- @brief Creates new book record
-- @returns new record id
CREATE OR REPLACE FUNCTION create_book(
  title VARCHAR,        -- Book title
  author INT,           -- Author id
  published TIMESTAMP   -- Publication date
) RETURNS INT AS $$
BEGIN
  INSERT INTO test_book (book_title, author_id, book_published)
  VALUES (title,author,published);
  RETURN lastval();
END;
$$ LANGUAGE 'plpgsql';


-- @brief Updates book information
CREATE OR REPLACE FUNCTION modify_book(
  id INT,               -- Book id
  title VARCHAR,        -- Book title
  author INT,           -- Author id
  published TIMESTAMP   -- Publication date
) RETURNS VOID AS $$
BEGIN
  UPDATE test_book 
  SET book_title = title, author_id = author, book_published = published 
  WHERE book_id = id;
END;
$$ LANGUAGE 'plpgsql';

-- @brief Deletes book record
CREATE OR REPLACE FUNCTION delete_book(
  id INT                 -- Book id
) RETURNS VOID AS $$
BEGIN
  DELETE FROM test_book WHERE book_id = id;
END;
$$ LANGUAGE 'plpgsql';
