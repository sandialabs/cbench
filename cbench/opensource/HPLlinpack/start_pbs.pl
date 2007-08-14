#!/usr/bin/perl -w

#@run_sizes = (1,2,4,8,9,16,25,32,36,49,64,81,100,121);
@run_sizes = (1,2,4,8,9,16,25,32,36,49,64,81,100,121,128,144,169,196,200,225,256,289,324,361,400,441,462);

$num_run_sizes = @run_sizes;
$system_name = "iccpbs";

for ($i = $num_run_sizes - 1; $i; $i--) {
    $name = $system_name.".". $run_sizes[$i];
    chdir $name;
    print "qsub hpl-$run_sizes[$i].pbs\n";
    system "qsub -V hpl-$run_sizes[$i].pbs";
    sleep 1;
    chdir "..";
}
