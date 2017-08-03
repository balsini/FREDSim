<?php
    require 'utility.php';
    
    $root_dir = "results/";
                
    $simulations = scan_dir($root_dir);
            
    $simul_id = 0;
    if (isset($_REQUEST['eid']) && $_REQUEST['eid'] < count($simulations)) {
        $simul_id = $_REQUEST['eid'];
    }
            
    $simulation = $root_dir . $simulations[$simul_id] . "/";
?>

<form id="plot_form">
    <div class="top">
        <label>Simulation</label>
        <select name="eid" id="eid" onchange="changeExperiment()">
        <?php
            $counter = 0;
            foreach ($simulations as $s) {
                echo "<option value=\"" . $counter . "\"";
                
                if ($counter == $simul_id)
                    echo " selected";
                
                echo ">" . $s . "</option>";
                $counter = $counter + 1;
            }
        ?>
    
        </select>
        <div id="dimContainer" style="visibility:hidden;">
            <label>Dimensions</label>
            <select name="dimensions" id="dimensions" onchange="changeDimensions()">
                <option value="d2">2D</option>
            </select>
        </div>
        <div id="progress"></div>
        <div id="progressBar" class="jquery-ui-like"><div></div></div>
    </div>
    
    <?php
        explore_simul_param($simulation);
    ?>
    
    <div class="param">
        <input type="checkbox" name="sw" class="swCheckbox" checked="checked"> Show SW<br />
        <input type="checkbox" name="hw" class="hwCheckbox" checked="checked"> Show HW<br />
        <input type="checkbox" name="an" class="anCheckbox"> Show AN<br />
    </div>
    
    <div class="param">
        <input type="button" value="Plot" class="param" id="plotButton" onclick="getData()">
        <img id="loadingImg" src="img/loading.gif" alt="Loading" style="width:50px;height:50px;display:none;">
    </div>
</form>

<div id="container" style="display: none; min-width: 95%; height: 60%; margin: 0 auto"></div>

<div class="simulInfo" id="containerCSV" style="display: none;"><pre></pre></div>

<div class="simulInfo" id="simulInfo">
<?php
    $simulInfo = $simulation . "info.xml";
    
    if (file_exists ( $simulInfo )) {
        $myfile = fopen($simulInfo, "r") or die("Unable to open file!");
        echo "<pre>" . htmlentities(fread($myfile,filesize($simulInfo))) . "</pre>";
        fclose($myfile);
    }
?>
</div>
    
<script>
    var maxDimensions = 3;
    var checkedRanges = 0;
    var intervalDelay = 5000;
    var intervalID;
    
    function paramsVisibility(v) {
        var x = document.getElementsByClassName("param");
        
        var i;
        for (i = 0; i < x.length; i++) {
            x[i].style.visibility = v;
        }
    }
    
    function masterVisibility(v) {
        var x = document.getElementsByClassName("masterCheckboxContainer");
        var i;
        
        if (v) {
            var r = document.getElementsByClassName("rangeCheckbox");
            
            for (i = 0; i < x.length; i++) {
                if (r[i].checked == true) {
                    x[i].style.display = "block";
                }
            }
        } else {
            for (i = 0; i < x.length; i++) {
                x[i].style.display = "none";
            }
        }
    }
    
    function rangeVisibilityUpdate(v) {
        var r = document.getElementsByClassName("rangeCheckbox");
        if (v) {
            for (i = 0; i < r.length; i++) {
                if (r[i].checked == true) {
                    r[i].parentNode.style.visibility = "visible";
                } else {
                    r[i].parentNode.style.visibility = "hidden";
                }
            }
        } else {
            for (i = 0; i < r.length; i++) {
                r[i].parentNode.style.visibility = "visible";
            }
        }
    }
    
    function checkExperimentProgress() {
        //alert("ciao");
        
        var myselect = document.getElementById("eid");
        var data_out = "eid=".concat(myselect.selectedIndex.toString());
        
        $.ajax({
            url: "checkStatus.php",
            dataType: 'json',
            success: function(data){
                var progressselect = document.getElementById("progress");
                var progressBarselect = document.getElementById("progressBar");
                if (data[0] == data[1]) {
                    clearInterval(intervalID);
                    paramsVisibility("visible");
                    progressselect.innerHTML = "";
                    
                    progressBarselect.style.display = "none";
                } else {
                    paramsVisibility("hidden");
                    var percentNum = Math.ceil(data[0] / data[1] * 100);
                    var percent = (percentNum).toString().concat("% ");
                    var detail = "[".concat(data[1].toString()).concat(" - ").concat(data[0].toString()).concat(" = ").concat((data[1] - data[0]).toString()).concat("]");
                    //progressselect.innerHTML = percent.concat(detail);
                    progressselect.innerHTML = detail;
                    
                    progressBar(percentNum, $('#progressBar'));
                    progressBarselect.style.display = "block";
                }
            },
            data:   data_out
        });
    }
    
    function init() {
        checkExperimentProgress();
        intervalID = setInterval(checkExperimentProgress, intervalDelay);
    }
    
    function changeExperiment() {
        var myselect = document.getElementById("eid");
        window.location.href = "./?eid=".concat(myselect.selectedIndex.toString());
    }
    
    function rangeShower(caller) {
        var changed_id = caller.name;
        var myselect = document.getElementById(changed_id);
        if (caller.checked) {
            if (checkedRanges + 1 < maxDimensions) {
                checkedRanges = checkedRanges + 1;
                myselect.style.visibility = "visible";
            } else {
                caller.checked = false;
            }
        } else {
            checkedRanges = checkedRanges - 1;
            myselect.style.visibility = "hidden";
        }
        
        var dimselect = document.getElementById("dimContainer");
        
        if (checkedRanges >= 2) {
            dimselect.style.visibility = "visible";
            masterVisibility(true);
            
            var x = document.getElementsByClassName("masterCheckbox");
            for (i = 0; i < x.length; i++) {
                var targetName = caller.name.concat("m");
                if (x[i].name == targetName) {
                    masterSelect(x[i]);
                }
            }
            
            rangeVisibilityUpdate(true);
        } else {
            dimselect.style.visibility = "hidden";
            masterVisibility(false);
            
            rangeVisibilityUpdate(false);
        }
    }
    
    function masterSelect(m) {
        var x = document.getElementsByClassName("masterCheckbox");
        for (i = 0; i < x.length; i++) {
            x[i].checked = false;
        }
        m.checked = true;
    }
    
    function getData() {
    
        /*var data_out = {
            name: 'ciao'
        }*/
        
        var loadingImage = document.getElementById("loadingImg");
        loadingImage.style.display = "block";
        
        var plotbutton = document.getElementById("plotButton");
        plotbutton.style.display = "none";
        
        var data_out = $('form').serialize();
    
        $.ajax({
            url:    '/cgi-bin/perltest.pl',
            //url:    'aggregator.php',
            type:   'GET',
            dataType: 'json',
            success : function (data_in) {
                plot(data_in);
            },
            complete: function() {
                var loadingImage = document.getElementById("loadingImg");
                loadingImage.style.display = "none";
                
            var plotbutton = document.getElementById("plotButton");
            plotbutton.style.display = "block";
            },
            data:   data_out
        });
    }
</script>