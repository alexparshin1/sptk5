<?php

header('Content-Type: text/json');
header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: POST, GET, OPTIONS");
header("Access-Control-Allow-Headers: Content-Type, Content-Length, Content-Encoding, Access-Control-Allow-Origin, Authorization");

$crs = pg_pconnect('host=localhost dbname=sptk user=sptk password=swinka01');
$SQL = "select hnl_date, hnl_version,hnl_description from host_news_list where not hnl_version is null and hnl_date > CURRENT_DATE - interval '12 months' order by 1 desc";
$rs = pg_query($crs,$SQL);

echo "[\n";
$first = true;
while ($result = pg_fetch_row($rs)) {

   if ($first) {
      $first = false;
   } else {
      echo(",\n");
   }

   $version_date = $result[0];
   $version_name = $result[1];
   $version_descr = $result[2];

   echo("{ \"version_date\": \"$version_date\", ");
   echo("\"version\": \"$version_name\", ");

   $version_descr = stripslashes($version_descr);
   $version_descr = preg_replace("/<ul>/","<ul class=\"news\">",$version_descr);
   $version_descr = preg_replace("/[\r\n]+/","\\n",$version_descr);
   $version_descr = preg_replace("/\"/","'",$version_descr);
   echo("\"news\": \"" .  $version_descr . "\"}");
}

echo "]";
?>
