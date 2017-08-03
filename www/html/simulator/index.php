<html>
 <head>
  <title>Simulation Visualizer (Alpha 1.1)</title>
  <script src="https://code.jquery.com/jquery-2.2.3.min.js"></script>
  
  <script type="text/javascript" src="progressbar.js"></script>
  <link rel="stylesheet" type="text/css" href="skins/tiny-green/progressbar.css">
  <link rel="stylesheet" type="text/css" href="skins/jquery-ui-like/progressbar.css">
 </head>
 
 <style>
input[type=text], select {
    margin : 2px 2px 2px 2px;
    display : block;
    border-radius : 4px;
}

input[type=button] {
    background-color : #4CAF50;
    color : white;
    margin : 8px 0;
    border : none;
    border-radius : 4px;
    cursor : pointer;
    display : block;
    visibility: hidden;
}

input[type=button]:hover {
    background-color: #45a049;
}

div.param {
    display: inline-block;
    border-radius: 5px;
    background-color: #f2f2f2;
    margin : 3px 3px 3px 3px;
    padding: 8px;
    visibility: hidden;
    vertical-align : top;
}

div.top {
    display: inline-block;
    border-radius: 5px;
    background-color: #f2f2f2;
    margin : 3px 3px 3px 3px;
    padding: 8px;
    vertical-align : top;
}

div.loading {
    display: inline-block;
    border-radius: 5px;
    background-color: #f2f2f2;
    margin : 3px 3px 3px 3px;
    padding: 8px;
    vertical-align : top;
}

div.simulInfo {
    border-radius: 5px;
    background-color: #f2f2f2;
    margin : 3px 3px 3px 3px;
    padding: 0px;
    vertical-align : top;
}

#progressBar {
    width: 100%;
    height: 22px;
    border: 1px solid #111;
    background-color: #292929;
}

#progressBar div {
    height: 100%;
    color: #fff;
    text-align: right;
    line-height: 22px; /* same as #progressBar height if we want text middle aligned */
    width: 0;
    background-color: #0099ff;
}
</style>
 
<body onload="init()">

<?php

include 'chooser.php';
include 'plot.php';

?>
</body>
</html>