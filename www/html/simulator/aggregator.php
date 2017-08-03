<?php
    $x = array("Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec");
    
    $value0 = array([
                "name" => "FP_PREEMPTIVE",
                "data" => array(1,2,3,2,5)
                ]);
    /*
    for ($i=0; i<1; i = i+0.01) {
        array_push($y, i);
    }
    */
    $data = ["x" => $x,
            "data" => $value0];
    
    echo json_encode($data);
?>