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

$date = isset($data['date']) ? $data['date'] : "";
$title = isset($data['title']) ? $data['title'] : "";
$news = isset($data['news']) ? $data['news'] : "";

if (!preg_match('/^\d{4}-\d{2}-\d{2}$/', $date)) {
   reply(false, "Invalid date");
}
if ($title === "" || $news === "") {
   reply(false, "Title and news text are required");
}

$crs = pg_pconnect('host=localhost dbname=sptk user=sptk password=swinka01');
if (!$crs) {
   reply(false, "Database is not available");
}

$rs = pg_query_params($crs,
   "insert into host_news_list (hnl_date, hnl_version, hnl_description) values ($1, $2, $3)",
   array($date, $title, $news));
if (!$rs) {
   reply(false, "Can't insert news record");
}

reply(true, "");
?>
