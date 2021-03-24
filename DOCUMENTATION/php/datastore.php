<?php

$device = "default";

$device = $_GET['device'];

$data = file_get_contents("php://input");
$method=getenv('REQUEST_METHOD');

$my_file = $device.'.csv';
$handle = fopen($my_file, 'a') or die('Cannot open file:  '.$my_file);
fwrite($handle, $data);
fclose($handle);

echo "received and stored data in file $my_file";

?>