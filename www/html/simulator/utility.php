<?php
function scan_dir($dir) {
    $ignored = array('.', '..', '.svn', '.htaccess');

    $files = array();
    foreach (scandir($dir) as $file) {
        if (!is_dir($dir . "/" . $file) || in_array($file, $ignored))
            continue;
        $files[$file] = filemtime($dir . '/' . $file);
    }
    
    #arsort($files);
    $files = array_keys($files);

    return $files;
}

function dirs2form($dirs) {
    foreach ($dirs as &$value) {
        echo "Directory: " . $value . "\n";
    }
}

function dir2value($dir) {
    return substr(strrchr($dir, "_"), 1);
}

function reorderValues($values) {
    $files = array();
    foreach ($values as &$v) {
        $files[$v] = (float)dir2value($v);
    }
    
    asort($files);
        
    $files = array_keys($files);

    return $files;
}

function str_lreplace($search, $replace, $subject)
{
    $pos = strrpos($subject, $search);

    if($pos !== false) {
        $subject = substr_replace($subject, $replace, $pos, strlen($search));
    }

    return $subject;
}

function dir2param($dir) {
    return str_lreplace(substr(strrchr($dir, "_"), 0), "", $dir);
}

function explore_simul_param($base) {
    $pars = scan_dir($base);
    
    if (count($pars) == 0)
        return;
    if (strlen(dir2value($pars[0])) == 0)
        return;

    $pars = reorderValues($pars);
        
    echo "<div class=\"param\">";
    echo "<label>" . dir2param($pars[0]) . "</label>\n";
    echo "<select name=\"" . dir2param($pars[0]) . "_min\">\n";
    //echo "Param: " . dir2param($pars[0]) . "\n";
    
    $counter = 0;
    
    foreach ($pars as &$par) {
        
        
        echo "<option value=\"" . dir2value($par) . "\"";
        if ($counter == 0) {
            echo " selected";
        }
        echo ">" . dir2value($par) . "</option>\n";
        $counter = $counter + 1;
        //echo "Value:   " . dir2value($par) . "\n";
    }
    
    echo "</select>\n";
    
    if (count($pars) > 1) {
        echo "<select style=\"visibility:hidden;\" name=\"";
        echo dir2param($pars[0]) . "_max\"id=\"" . dir2param($pars[0]) . "_max_c\">\n";
        //echo "Param: " . dir2param($pars[0]) . "\n";
        
        $counter = 0;
        
        foreach ($pars as &$par) {
            echo "<option value=\"" . dir2value($par) . "\"";
            if ($counter == count($pars) - 1) {
                echo " selected";
            }
            echo ">" . dir2value($par) . "</option>\n";
            $counter = $counter + 1;
            //echo "Value:   " . dir2value($par) . "\n";
        }
        
        echo "</select>\n";
    
        
        echo "<div class=\"rangeCheckboxContainer\"><input type=\"checkbox\" class=\"rangeCheckbox\" name=\"";
        echo dir2param($pars[0]) . "_max_c";
        echo "\" onChange=\"rangeShower(this)\"> Range</div>";
        
        echo "<div style=\"display: none;\" id=\"" . dir2param($pars[0]) . "_max_c\" class=\"masterCheckboxContainer\"><input type=\"checkbox\" name=\"";
        echo dir2param($pars[0]) . "_max_cm";
        echo "\" class=\"masterCheckbox\" onChange=\"masterSelect(this)\"> Master</div>";
    }
    
    echo "</div>";
    explore_simul_param($base . "/" . $pars[0]);
}

function explore_simul_tree($base) {
    $base = $base . "/";
    
    $experiments = scan_dir($base);
    $counter = 0;

    $default_exp = 0;
    if (isset($_GET["exp"]))
        $default_exp = $_GET["exp"];

    foreach ($experiments as &$experiment) {
        explore_simul_param($base . $experiment);
    }
}

?>