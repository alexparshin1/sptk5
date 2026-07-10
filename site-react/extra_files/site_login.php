<?php

header('Content-Type: text/json');
header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: POST, GET, OPTIONS");
header("Access-Control-Allow-Headers: Content-Type, Content-Length, Content-Encoding, Access-Control-Allow-Origin, Authorization");

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
   exit;
}

function reply($success, $description)
{
   echo json_encode(array("result" => array("success" => $success, "description" => $description)));
   exit;
}

$data = json_decode(file_get_contents('php://input'), true);
if (!$data) {
   reply(false, "Invalid request");
}

$username = isset($data['username']) ? trim($data['username']) : "";
$secret = isset($data['secret']) ? $data['secret'] : "";

if ($username === "" || $secret === "") {
   reply(false, "Username and password are required");
}

$crs = pg_pconnect('host=localhost dbname=sptk user=sptk password=swinka01');
if (!$crs) {
   reply(false, "Database is not available");
}

pg_query($crs,
   "create table if not exists users (" .
   "id serial primary key, " .
   "username varchar(255) not null unique, " .
   "password varchar(255) not null)");

$rs = pg_query_params($crs, "select password from users where username = $1", array($username));
if (!$rs) {
   reply(false, "Can't read user record");
}

$row = pg_fetch_row($rs);
if (!$row) {
   // Only a single admin account is allowed: accept and store the credentials
   // on the initial login, while no user exists yet
   $rs = pg_query($crs, "select count(*) from users");
   $count = $rs ? pg_fetch_row($rs) : null;
   if (!$count || $count[0] > 0) {
      reply(false, "Invalid username or password");
   }

   $hash = password_hash($secret, PASSWORD_DEFAULT);
   $rs = pg_query_params($crs, "insert into users (username, password) values ($1, $2)",
                         array($username, $hash));
   if (!$rs) {
      reply(false, "Can't create user record");
   }
   reply(true, "Initial login: credentials stored");
}

if (!password_verify($secret, $row[0])) {
   reply(false, "Invalid username or password");
}

reply(true, "");
?>
