-- @brief Creates new author record
-- @returns new record id
CREATE OR REPLACE FUNCTION create_author(
  name VARCHAR          -- author name
) RETURNS INT AS $$
BEGIN
  INSERT INTO test_author (author_name)
  VALUES (name);
  RETURN lastval();
END;
$$ LANGUAGE 'plpgsql';


-- @brief Updates author information
CREATE OR REPLACE FUNCTION modify_author(
  id INT,               -- author id
  name VARCHAR          -- author name
) RETURNS VOID AS $$
BEGIN
  UPDATE test_author 
  SET author_name = name
  WHERE author_id = id;
END;
$$ LANGUAGE 'plpgsql';

-- @brief Deletes author record
CREATE OR REPLACE FUNCTION delete_author(
  id INT                 -- author id
) RETURNS VOID AS $$
BEGIN
  DELETE FROM test_book WHERE author_id = id;
  DELETE FROM test_author WHERE author_id = id;
END;
$$ LANGUAGE 'plpgsql';
