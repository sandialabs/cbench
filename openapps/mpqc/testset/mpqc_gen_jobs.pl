#!/usr/bin/perl

###############################################################################
#    Copyright (2004) Sandia Corporation.  Under the terms of Contract
#    DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
#    certain rights in this software
#
#    This file is part of Cbench.
#
#    Cbench is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    any later version.
#
#    Cbench is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with Cbench; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
###############################################################################


# need to know where everything cbench lives!
$BENCH_HOME = $ENV{CBENCHOME} ? $ENV{CBENCHOME} :
    "$ENV{HOME}\/cbench";

unshift @INC, "$BENCH_HOME";
require "cbench.pl";

use Getopt::Long;

GetOptions(
	'mpqcbin=s' => \$mpqcbin,
	'ident=s' => \$ident,
	'debug:i' => \$DEBUG,
	'help' => \$help,
);

if (defined $help) {
	usage();
	exit;
}

$testset = 'mpqc';
$bench_test = get_bench_test();
$testset_path = "$bench_test/$testset";
$DEBUG and print "DEBUG: $bench_test $testset_path\n";

(!defined $ident) and $ident = $cluster_name . "1";
$binpath = "$testset_path";
(!defined $mpqcbin) and $mpqcbin = 'mpqc';

$walltime = "00:30:00";

# read in the appropriate batch system header template
$file = "$bench_test\/$batch_method\_header.in";
open (IN,"<$file") or die
	"Could not open $file ($!)";
undef $/;
$batch_header = <IN>;
close(IN);
$/ = "\n";

# read in the appropriate interactive job header template
$file = "$bench_test\/interactive_header.in";
open (IN,"<$file") or die
	"Could not open $file ($!)";
undef $/;
$interactive_header = <IN>;
close(IN);
$/ = "\n";

# Here we build the core template(s) we will use to do all our
# substitutions below. We look in the test set directory for files
# named TESTSET_BINARYNAME.in where TESTSET is the test set name,
# i.e. 'bandwidth', 'xhpl', and BINARYNAME is the name of the specific
# benchmark to be run
@temp = `/bin/ls -1 $testset_path\/$testset\_*\.in`;

%templates = ();
foreach (@temp) {
	chomp $_;
	(/(\S+)\/($testset)_(\S+)\.in/) or next;
	
	# start building the job templates
	$templates{"$3"}{'batch'} = $batch_header;
	$templates{"$3"}{'interactive'} = $interactive_header;

	# read in the job template so we can add it
	$file = "$testset_path\/$2\_$3.in";
	open (IN,"<$file") or die
		"Could not open $file ($!)";
	undef $/;
	$job_template = <IN>;
	close(IN);
	$/ = "\n";

	# continue building the job templates
	$templates{"$3"}{'batch'} .= $job_template;
	$templates{"$3"}{'interactive'} .= $job_template;
	
	if ($DEBUG and $DEBUG < 2) {
		print "DEBUG: found and processed job template $2\_$3.in\n";
	}
	elsif ($DEBUG > 1) {
		print "DEBUG:  batch job template $2\_$3.in\n" .
			"====================================================\n".
			$templates{"$3"}{'batch'} .
			"====================================================\n";
	}
}


# This is the core of the set of loops that generate all the
# job files (batch and interactive) for all the permutations of
# number of processors, ppn, and specific test.
#
# The method cbench uses is to build a template for all the tests
# and then do keyword replacment in the template to generate the
# actual job files.

# make sure the directory is there with the proper test identification
(! -d "$testset_path\/$ident") and mkdir "$testset_path\/$ident",0750;

# outer loop iterates over the various ppn cases as defined in 
# the max_ppn_procs hash in cluster.def
foreach $ppn (keys %max_ppn_procs) {
	# inner loop iterates of the various run sizes (i.e. number of
	# processors in a parallel job) in the @run_sizes array as
	# defined in cbench.pl
	foreach $numprocs (@run_sizes) {
		# check and make sure we don't generate jobs over the max
		# number of procs for the cluster or for the current
		# ppn (as specified in cluster.def)
		($numprocs > $max_ppn_procs{$ppn}) and next;
		($numprocs > $max_procs) and next;

		# iterate over the job templates we need to process
		foreach $job (keys %templates) {
			# iterate over batch and interactive job generation
			foreach $runtype (('batch','interactive')) {
				my $captype = uc $runtype;

				# figure out how many nodes we need based on the number of
				# procs and ppn
				$numnodes = calc_num_nodes($numprocs,$ppn);

				# this check should be superflous but do it anyway
				($numnodes > $max_nodes) and next;

				# build the full job name
				$jobname = "$job-".$ppn."ppn-$numprocs";

				# prime the output buffers with the appopriate template
				$outbuf = $templates{$job}{$runtype};

				# here we do all the standard substitutions
				$outbuf = std_substitute($outbuf,$numprocs,$ppn,$numnodes,
							$runtype,$walltime,$testset,$jobname);

				# custom substitutions
				$outbuf =~ s/BIN_PATH_HERE/$binpath/gs;
				$outbuf =~ s/MPQC_BIN_HERE/$mpqcbin/gs;

				# build the filename
				($runtype eq 'batch') and $outfile = "$jobname\.pbs";
				($runtype eq 'interactive') and $outfile = "$jobname\.sh";

				($DEBUG) and print "Writing $captype script for $jobname\n";

				# write out the generated job file
				open (OUT,">$testset_path\/$ident\/$outfile") or die
					"Could not write $testset_path\/$ident\/$outfile ($!)";
				print OUT $outbuf;
				close(OUT);

				($runtype eq 'interactive') and
					`/bin/chmod gu+x $testset_path\/$ident\/$outfile`;
			}
		}
	}
}

sub usage {
    print "USAGE: $0 \n";
    print "Cbench script to generate jobs in the $testset test set\n".
	  "   --mpqcbin      use the specified rotate binary\n".
          "   --ident          identifying string for the test\n".
	  "   --debug <level>  turn on debugging at the specified level\n";
}
