<?php
require 'utility.php';

$root_dir = "results/";
                
$simulations = scan_dir($root_dir);

function getLines($file)
{
    $f = fopen($file, 'rb');
    $lines = 0;

    while (!feof($f)) {
        $lines += substr_count(fread($f, 8192), "\n");
    }

    fclose($f);

    return $lines;
}

$simul_id = 0;
if (isset($_REQUEST['eid']) && $_REQUEST['eid'] < count($simulations)) {
    $simul_id = $_REQUEST['eid'];
}

$simulation = $root_dir . $simulations[$simul_id] . "/";

$max_file = $simulation . "progressTotal.txt";
$max = fopen($max_file, "r")
    or die("Unable to open file!");
$max_value = fread($max,filesize($max_file));
fclose($max);

$progress_file = $simulation . "progressDone.txt";
$progress_value = getLines($progress_file);

print json_encode(array($progress_value, $max_value));

?>