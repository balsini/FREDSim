#!/usr/bin/perl

use strict;
#use warnings;

use CGI;
use JSON;

use Cwd;
use File::Slurp qw(read_dir);

#-------------------------------------------------------------

my $base_path = "../html/simulator/results/";

my $filename = 'output.txt';

my $dbgString = "Debugging: ";

my %results = ();

my $cgi = CGI->new();

sub deadline_miss_file
{
    my $relative_RT_column_HW = 3;
    my $relative_RT_column_SW = 6;
    my $analysis_column = 7;
    my $fh = $_[0]; # file pointer
    
    my $SW_miss = 0;
    my $HW_miss = 0;
    my $A_miss = 1;
    
    while (my $row = <$fh>) {
        # Remove comments
        my $comment_start = index($row, '#'); 
        $row = substr($row, 0, $comment_start);
        
        my @columns = split /\s+/, $row;
        
        if (scalar(@columns) ge 1 &&
            (scalar(@columns) le $relative_RT_column_SW || scalar(@columns) le $relative_RT_column_HW)) {
            print 'here';
        }
        
        if (scalar(@columns) ge $relative_RT_column_HW && scalar(@columns) ge $relative_RT_column_SW) {
            
            if ($columns[$relative_RT_column_SW] gt '1.0') {
                $SW_miss = 1;
            }
            if ($columns[$relative_RT_column_HW] gt '1.0') {
                $HW_miss = 1;
            }
            
            
            if (scalar(@columns) ge $analysis_column) {
                if ($columns[$analysis_column] ge '1') {
                    $A_miss = 0;
                }
            }
        }
    }
    return ($HW_miss, $SW_miss, $A_miss);
}

sub read_dir_sorted_name
{
    my $dir = $_[0];
    my @res = (sort { $a cmp $b } grep { -d "$dir/$_" } read_dir($dir));
}

sub read_dir_sorted_value
{
    my $dir = $_[0];
    my @dir_names = grep { -d "$dir/$_" } read_dir($dir);
    my @dir_names_ordered = ();
    
    my %dir_hash = ();
    
    for my $x (@dir_names) {
        $dir_hash{ dir2param($x) . "_" . dir2value($x) } = dir2value($x);
    }
    
    foreach my $name (sort { $dir_hash{$a} <=> $dir_hash{$b} } keys %dir_hash) {
        push(@dir_names_ordered, $name);
    }
    
    return (@dir_names_ordered);
}

sub read_dir_sorted_modif
{
    my $dir = $_[0];
    my @res = (sort { -M "$dir/$a" <=> -M "$dir/$b" } grep { -d "$dir/$_" } read_dir($dir));
}

my @experiments = read_dir_sorted_name($base_path);



my %param = ();
$param{"eid"} = $cgi->param('eid');



if ($param{"eid"} >= scalar(@experiments)) {
    $param{"eid"} = 0;
}
my $root = $base_path . $experiments[$param{"eid"}]; 

sub dir2param
{
    my $dir = $_[0];
    
    my $last_underscore = rindex($dir, "_");
    my $param = substr($dir, 0, $last_underscore);
    
    return ($param);
}

sub dir2value
{
    my $dir = $_[0];
    
    my $last_underscore = rindex($dir, "_");
    my $value = substr($dir, $last_underscore + 1);
    
    return ($value);
}

my @parameters = ();

sub explore_simul_tree_get_parameters
{
    my $base = $_[0];
    
    my @pars = read_dir_sorted_name($base);
    
    if (scalar(@pars) == 0) {
        return;
    }
    
    if (length(dir2param($pars[0])) == 0) {
        return;
    }
    
    #print dir2param($pars[0]) . " " . dir2value($pars[0]) . " " . scalar(@pars) . "\n";
    
    push(@parameters, dir2param($pars[0]));
    
    my $next_dir = $base . "/" . $pars[0];
    explore_simul_tree_get_parameters($next_dir);
}

my $switched_master = 0;

my $last_range_param_found_name;

sub explore_simul_tree_chosen_folders
{
    my $dir = $_[0] . "/";
    my $index = $_[1];
    my $par1 = $_[2];
    my $val1 = $_[3];
    my $par2 = $_[4];
    my $val2 = $_[5];
    
    if ($index >= scalar(@parameters)) {
        #print "leaf: " . $dir . "\n";
        
        if (length($par2) == 0) {
            $val2 = 0;
        }
        
        $results{ $dir } = [$par1, $val1, $par2, $val2, 0, 0, 0];
                            
        #print "inserted: $par1\t$val1\t$par2\t$val2\t0\t0\n";
        
        return;
    }
    
    my $checkboxName = $parameters[$index] . "_max_c";
    my $fieldName = $parameters[$index] . "_min";
    
    #print $parameters[$index] . "\n";
    
    if (exists($param{ $checkboxName })) {
        #print "checkbox found: " . $checkboxName . "\n";
        
        my $param_index;
        
        if (length($par1) == 0) {
            $par1 = $parameters[$index];
            $param_index = 1;
            #print "updated parameter1: " . $par1 . "\n";
        } else {
            if (length($par2) == 0) {
                $par2 = $parameters[$index];
                $last_range_param_found_name = $par2;
                $param_index = 2;
                #print "updated parameter2: " . $par2 . "\n";
            }
        }
        
        my $fieldNameMax = $parameters[$index] . "_max";
        
        my $dir_min = $parameters[$index] . "_" . $param{ $fieldName };
        my $dir_max = $parameters[$index] . "_" . $param{ $fieldNameMax };
    
        #print "dir_min: " . $dir_min . "\n";
        
        my $running = 0;
        
        for my $x (read_dir_sorted_value($dir)) {
        
            #print "  current: " . $x . "\n";
            
            if (!$running) {
                if ($x eq $dir_min) {
                    $running = 1;
                    #print "found min\n";
                } else {
                    next;
                }
            }
            
            if ($param_index == 1) {
                $val1 = dir2value($x);
                #print "updated value1: " . $val1 . "\n";
            } else {
                $val2 = dir2value($x);
                #print "updated value2: " . $val2 . "\n";
            }
            
            my $new_dir = $dir . $x;
            #print "entering dir: " . $new_dir . "\n";
            explore_simul_tree_chosen_folders($new_dir, $index + 1, $par1, $val1, $par2, $val2);
            
            if ($x eq $dir_max) {
                #print "found max\n";
                last;
            }
        }
        #print "dir_max: " . $dir_max . "\n";
    } else {
        my $new_dir = $dir . $parameters[$index] . "_" . $param{ $fieldName };
    
        #print "entering dir: " . $new_dir . "\n";
        explore_simul_tree_chosen_folders($new_dir, $index + 1, $par1, $val1, $par2, $val2);
    }
}

explore_simul_tree_get_parameters($root);

for my $p (@parameters) {
    my $full_name;
    
    $full_name = $p."_min";
    $param{$full_name} = $cgi->param($full_name);
    
    $full_name = $p."_max";
    $param{$full_name} = $cgi->param($full_name);
    
    $full_name = $p."_max_cm";
    $param{$full_name} = $cgi->param($full_name);
    
    $full_name = $p."_max_c";
    
    my $foo = $cgi->param($full_name); 
    if (defined $foo) {
        $param{$full_name} = $cgi->param($full_name);
    }
}

=pod
$param{"eid"} = "0";
$param{"dimensions"} = "d2";
$param{"FRI_min"} = "2";
$param{"FRI_max"} = "3";
$param{"SPEEDUP_min"} = "0.100000";
$param{"SPEEDUP_max"} = "1.900000";
$param{"U_SW_min"} = "0.100000";
$param{"U_SW_max"} = "0.900000";
$param{"U_SW_max_c"} = "on";
$param{"U_HW_min"} = "0.100000";
$param{"U_HW_max"} = "0.900000";
$param{"U_HW_max_c"} = "on";
$param{"U_HW_max_cm"} = "on";
#$param{"U_SW_max_cm"} = "on";
#print "\n\n";
=cut



explore_simul_tree_chosen_folders($root, 0, "", "", "", "", 0);

#print "primo trovato: ".$fist_range_param_found_name."\n";
#print "default cercato: ". $fist_range_param_found_name."_max_cm\n";
#print "in db: ". $param{$fist_range_param_found_name."_max_cm"}."\n";

if ($param{$last_range_param_found_name."_max_cm"} eq "on") {
    $switched_master = 1;
} else {
    $switched_master = 0;
}

#print "\n\n";

foreach my $experiment_root (keys %results) {
    #print "\tx: $x\n";
    
    my $dlmHW = 0;
    my $dlmSW = 0;
    my $dlmA = 0;
    my $samples = 0;
    
    #print "Checking experiment $experiment_root\n";
    #print "parameters: ". $results{ $experiment_root }[0] . "\n";
    #print "parameters: ". $results{ $experiment_root }[1] . "\n";
    #print "parameters: ". $results{ $experiment_root }[2] . "\n";
    #print "parameters: ". $results{ $experiment_root }[3] . "\n";

    for my $x (grep { -d "$experiment_root/$_" } read_dir($experiment_root)) {
        #print "\tx: $x\n";
        
        my $file_path = $experiment_root . $x . '/' . $filename;
        
        #print "Checking file $file_path\n";
                
        open(my $fh, '<:encoding(UTF-8)', $file_path)
            or die "Could not open file '$file_path' $!";
            
        my @dl_miss = deadline_miss_file($fh);
        $dlmHW = $dlmHW + $dl_miss[0];
        $dlmSW = $dlmSW + $dl_miss[1];
        $dlmA = $dlmA + $dl_miss[2];
        $samples = $samples + 1;
    }
    
    my $succeededHW = ($samples - $dlmHW) / $samples;
    my $succeededSW = ($samples - $dlmSW) / $samples;
    my $succeededA = ($samples - $dlmA) / $samples;
    #print "$succeededHW\t$succeededSW\n";
    
    $results{ $experiment_root }[4] = $succeededHW;
    $results{ $experiment_root }[5] = $succeededSW;
    $results{ $experiment_root }[6] = $succeededA;
}

my $master;
my $slave;

if ($switched_master == 0) {
    $master = 0;
    $slave = 2;
} else {
    $master = 2;
    $slave = 0;
}

my $master_name;
my $slave_name;

my @x = ();
my @z = ();

my @matrix_ordered = ();
sub create_results_matrix
{
    my $x_index = $master + 1;
    my $z_index = $slave + 1;
    
    my %matrix = ();
    
    foreach my $e (keys %results) {
        $master_name = $results{ $e }[$master];
        $slave_name = $results{ $e }[$slave];
        
        my $x1 = $results{ $e }[$x_index];
        
        if (!grep { $_ eq $x1 } @x ) {
            push (@x, $x1);
        }
        
        my $z1 = $results{ $e }[$z_index];
        
        if (!grep { $_ eq $z1 } @z ) {
            push (@z, $z1);
        }
        
        $matrix { $x1 }{ $z1 }[0] = $results{ $e }[4];
        $matrix { $x1 }{ $z1 }[1] = $results{ $e }[5];
        $matrix { $x1 }{ $z1 }[2] = $results{ $e }[6];
    }
    
    @x = sort { $a <=> $b } @x;
    @z = sort { $a <=> $b } @z;
    
    my $row = 0;
    foreach my $zz (@z) {
        #print "$xx:\t";
        
        my $col = 0;
        foreach my $xx (@x) {
            $matrix_ordered[$row][$col][0] = $matrix { $xx }{ $zz }[0];
            $matrix_ordered[$row][$col][1] = $matrix { $xx }{ $zz }[1];
            $matrix_ordered[$row][$col][2] = $matrix { $xx }{ $zz }[2];
            
            $col = $col + 1;
        }
        $row = $row + 1;
        #print "\n";
    }
}

create_results_matrix();

sub print_results_matrix
{
    my $id = $_[0];
    print "Master: $master_name, Slave: $slave_name\n";
    foreach my $row (@matrix_ordered) {
        foreach my $element (@$row) {
            print @$element[$id], "\t";
        }
        print "\n";
    }
}

#print_results_matrix(0);
#print_results_matrix(1);

#print "\n\n";


#for my $x (read_dir_sorted_name($root)) {
#    print $x . "\n";
#}

# For each experiment 
=pod
for my $experiment (sort { $a cmp $b } grep { -d "$root/$_" } read_dir($root)) {
    print "$experiment\tSchedulableTasksetsHW\tSchedulableTasksetsSW\n";
    
    my $experiment_root = $root . '/' . $experiment;
    
    # For each experiment parameter
    for my $x (sort { $a cmp $b } grep { -d "$experiment_root/$_" } read_dir($experiment_root)) {
        #print "\tx: $x\n";
        print "$x\t";
        
        my $result_root = $experiment_root . '/' . $x;
        
        # For each result
        my $dlmHW = 0;
        my $dlmSW = 0;
        my $samples = 0;
        for my $result (sort { $a cmp $b } grep { -d "$result_root/$_" } read_dir($result_root)) {
                
                my $file_path = $result_root . '/' . $result . '/' .$filename;
                        open(my $fh, '<:encoding(UTF-8)', $file_path)
                          or die "Could not open file '$file_path' $!";
                          
                        my @dl_miss = deadline_miss_file($fh);
                        $dlmHW = $dlmHW + $dl_miss[0];
                        $dlmSW = $dlmSW + $dl_miss[1];
                        $samples = $samples + 1;
            }
            my $succeededHW = ($samples - $dlmHW) / $samples;
            my $succeededSW = ($samples - $dlmSW) / $samples;
            print "$succeededHW\t$succeededSW\n";
    }
}
=cut
#-------------------------------------------------------------


#my @x = ('Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec');

my %arr0 = ();
my %arr0SW = ();
my %arr0A = ();

foreach my $experiment_root (sort { $results{$a}[$master + 1] <=> $results{$b}[$master + 1] } keys %results) {
    #print "\tx: $x\n";
    
    my $x1 = $results{ $experiment_root }[$master + 1];
    my $x2 = $results{ $experiment_root }[$slave + 1];
    $slave_name = $results{ $experiment_root }[$slave];
    my $y = $results{ $experiment_root }[4];
    my $ySW = $results{ $experiment_root }[5];
    my $yA = $results{ $experiment_root }[6];
        
    push (@{$arr0{$x2}}, $y);
    push (@{$arr0SW{$x2}}, $ySW);
    push (@{$arr0A{$x2}}, $yA);
    #print "parameters: ". $results{ $experiment_root }[0] . "\n";
    #print "parameters: ". $results{ $experiment_root }[1] . "\n";

    #print $results{ $experiment_root }[4] . "\n";
}



###########################
# Prepare data for output #
###########################

my @value = ();
my $id;
my $count;

# HW
$id = 0;
$count = 0;

my $hw_defined = $cgi->param('hw');
if (defined $hw_defined) {

    foreach my $row (@matrix_ordered) {
        my %value0;
        my @name0;
        if (length($slave_name) == 0) {
            @name0 = "HW";
        } else {
            @name0 = "HW $slave_name: ". $z[$count];
        }
        $value0{"name"} = \@name0;
        
        my @out_array = ();
        
        foreach my $element (@$row) {
            push (@out_array, @$element[$id]);
        }
        
        $value0{"data"} = \@out_array;
        push (@value, \%value0);
        
        $count = $count + 1;
    }
}

$id = 1;
$count = 0;
my $sw_defined = $cgi->param('sw');
if (defined $sw_defined) {
    foreach my $row (@matrix_ordered) {
        my %value0;
        
        my @name0;
        if (length($slave_name) == 0) {
            @name0 = "SW";
        } else {
            @name0 = "SW $slave_name: ". $z[$count];
        }
        
        $value0{"name"} = \@name0;
        
        my @out_array = ();
        
        foreach my $element (@$row) {
            push (@out_array, @$element[$id]);
        }
        
        $value0{"data"} = \@out_array;
        push (@value, \%value0);
        
        $count = $count + 1;
    }
}

$id = 2;
$count = 0;
my $a_defined = $cgi->param('an');
if (defined $a_defined) {
    foreach my $row (@matrix_ordered) {
        my %value0;
        
        my @name0;
        if (length($slave_name) == 0) {
            @name0 = "A";
        } else {
            @name0 = "A $slave_name: ". $z[$count];
        }
        
        $value0{"name"} = \@name0;
        
        my @out_array = ();
        
        foreach my $element (@$row) {
            push (@out_array, @$element[$id]);
        }
        
        $value0{"data"} = \@out_array;
        push (@value, \%value0);
        
        $count = $count + 1;
    }
}



=pod
foreach my $x (sort { $a <=> $b } keys %arr0) {
    my %value0;
    my @name0 = "$slave_name: $x";
    $value0{"name"} = \@name0;
    $value0{"data"} = \@{$arr0{$x}};
    
    push (@value, \%value0);
}

foreach my $x (sort { $a <=> $b } keys %arr0SW) {
    my %value0;
    my @name0 = "$slave_name (SW): $x";
    $value0{"name"} = \@name0;
    $value0{"data"} = \@{$arr0SW{$x}};
    
    push (@value, \%value0);
}
=cut


my %result = ();
$result{"x"} = \@x;
$result{"data"} = \@value;

my @param = ($cgi->param('name'), getcwd, $dbgString);

my $simulName = "Simulated Schedulability Analysis";
$result{"simulName"} = $simulName;

$result{"xName"} = $master_name;
$result{"debug"} = \@param;
            
my $json = encode_json( \%result );

print $cgi->header( -type => 'application/json' );
print $json;

=pod
my %value0;
my @name0 = ('TB_PREEMPTIVE_TODO');
$value0{"name"} = \@name0;
$value0{"data"} = \@arr0;

my %value1;
my @arr1 = (3,2,5,4,5);
my @name1 = ('TB_NONPREEMPTIVE_TODO');
$value1{"name"} = \@name1;
$value1{"data"} = \@arr1;

my @value = (\%value0, \%value1);

my %result;
$result{"x"} = \@x;
$result{"data"} = \@value;

my @param = ($cgi->param('name'), getcwd, $dbgString);

$result{"debug"} = \@param;
            
my $json = encode_json( \%result );

print $cgi->header( -type => 'application/json' );
print $json;
=cut