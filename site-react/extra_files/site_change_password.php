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
$newSecret = isset($data['newSecret']) ? $data['newSecret'] : "";

if ($username === "" || $secret === "" || $newSecret === "") {
   reply(false, "Username, current and new passwords are required");
}

$crs = pg_pconnect('host=localhost dbname=sptk user=sptk password=swinka01');
if (!$crs) {
   reply(false, "Database is not available");
}

$rs = pg_query_params($crs, "select password from users where username = $1", array($username));
if (!$rs) {
   reply(false, "Can't read user record");
}

$row = pg_fetch_row($rs);
if (!$row || !password_verify($secret, $row[0])) {
   reply(false, "Invalid username or password");
}

$hash = password_hash($newSecret, PASSWORD_DEFAULT);
$rs = pg_query_params($crs, "update users set password = $2 where username = $1",
                      array($username, $hash));
if (!$rs) {
   reply(false, "Can't update user record");
}

reply(true, "Password changed");
?>
