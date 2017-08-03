<?php
 //echo exec('wc -l results/Mon_Apr_18_22:05:41_2016/progressDone.txt | cut -f1 -d\' \'');
?>
 over
 <?php
 //echo exec('cat results/Mon_Apr_18_22:05:41_2016/progressTotal.txt');
 //echo exec('./accelerated_full configs/example.xml');
?>

<?php

    // Request Post Variable
    $name = $_REQUEST['Name'];

    // Validation
    if($name == 'Adam') {
    echo json_error($_REQUEST['Name']);
    } else {
    echo json_success($_REQUEST['Name']);
    };

    // Return Success Function
    function json_success($msg) {
        $return = array();
        $return['error'] = FALSE;
        $return['msg'] = $msg;
        return json_encode($return);
    }

    // Return Error Function
    function json_error($msg) {
        $return = array();
        $return['error'] = TRUE;
        $return['msg'] = $msg;
        return json_encode($return);
    }

?>