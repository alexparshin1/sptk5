<?php

$crs = pg_pconnect('host=localhost dbname=sptk user=sptk password=swinka01');
$sql = "SELECT get_web_address_counter('" . $countername . "','" . getenv('REMOTE_ADDR') . "')";
$rs = pg_query($crs,$sql);
if (!$rs) {
    echo "Error\n";
    exit;
}

$rsa = pg_fetch_row($rs);
$viewers = $rsa[0];

echo("<span style='color:black;'>There are <span style='color:green;'><b> " . $viewers . "</b></span> unique visitors to this page. </span>");

?>