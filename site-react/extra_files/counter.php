<?php

$countername = $_GET['countername'];

$crs = pg_pconnect('host=localhost dbname=sptk user=sptk password=swinka01');
$sql = "SELECT get_web_address_counter('" . $countername . "','" . getenv('REMOTE_ADDR') . "')";
$rs = pg_query($crs,$sql);
if (!$rs) {
    echo "Error\n";
    exit;
}

$rsa = pg_fetch_row($rs);
$viewers = $rsa[0];

header('Content-Type: text/json');
header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: POST, GET, OPTIONS");
header("Access-Control-Allow-Headers: Content-Type, Content-Length, Content-Encoding, Access-Control-Allow-Origin, Authorization");

echo('{"visitors": ' . $viewers . "}");

?>