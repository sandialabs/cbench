#!/usr/bin/perl
# vim: syntax=perl tabstop=4
###############################################################################
#    Copyright (2005) Sandia Corporation.  Under the terms of Contract
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

# Generic Cbench *_gen_jobs.pl utility template script

# need to know where everything cbench lives!
use lib ($ENV{CBENCHOME} ? $ENV{CBENCHOME} : "$ENV{HOME}\/cbench");
$BENCH_HOME = $ENV{CBENCHOME} ? $ENV{CBENCHOME} :
    "$ENV{HOME}\/cbench";
require "cbench.pl";

# enable/disable color support appropriately
detect_color_support();

use Getopt::Long;
use Data::Dumper;
use Term::ANSIColor qw(:constants color colored);
$Term::ANSIColor::AUTORESET = 1;

# pre init some vars before command line parsing
my $xhplbin = 'xhpl';
my $hpccbin = 'hpcc';

# pass_through lets "non options" stay in ARGV w/o raising an error
Getopt::Long::Configure("pass_through");

GetOptions(
	'ident=s' => \$ident,
	'binident=s' => \$BINIDENT,
	'binname=s' => \$binname,
	'xhplbin=s' => \$xhplbin,
	'hpccbin=s' => \$hpccbin,
    'maxprocs=i' => \$maxprocs_cmdline,
    'minprocs=i' => \$minprocs_cmdline,
    'procs=i' => \$procs_cmdline,
    'runsizes=s' => \$runsizes,
	'testdir=s' => \$testdir,
	'jobcbenchtest=s' => \$JOBCBENCHTEST,
	'debug:i' => \$DEBUG,
	'help' => \$help,
	'redundant' => \$redundant,
);

if (defined $help) {
	usage();
	exit;
}

$testset = find_testset_identity($0);
$bench_test = get_bench_test();
$testset_path = "$bench_test/$testset";
$DEBUG and print "DEBUG: $bench_test $testset_path\n";

(!defined $ident) and $ident = $cluster_name . "1";
if (!defined $testdir) {
	$testdir = "$testset_path/$ident/TMP";
	print "$0: --testdir not specified, defaulting to $testdir\n";
	mkdir "$testdir",0750;
}


# custom generate processing control hash
%custom_gen_hash = (
	'benchmark' => {
		'xhpl' => {
			'init' => 'xhpl_gen_init',
			'innerloop' => 'xhpl_gen_innerloop',
		},
	},
	'testset' => {

	},
);

%templates = ();
build_job_templates($testset,\%templates);

#print Dumper(%custom_gen_hash);
#print Dumper(%templates);

# run initialization subroutines for an custom generation code
custom_gen_init($testset);

# This is the core of the set of loops that generate all the
# job files (batch and interactive) for all the permutations of
# number of processors, ppn, and specific test.
#
# The method cbench uses is to build a template for all the tests
# and then do keyword replacment in the template to generate the
# actual job files.

# make sure the directory is there with the proper test identification
(! -d "$testset_path\/$ident") and mkdir "$testset_path\/$ident",0750;

# if the user passed in a set of runsizes on the command line, convert
# to an array and replace the default @run_sizes array from cbench.pl
(defined $runsizes) and use_custom_runsizes($runsizes);

# outer loop iterates over the various ppn cases as defined in
# the max_ppn_procs hash in cluster.def
foreach $ppn (sort {$a <=> $b} keys %max_ppn_procs) {
	# prevent oversubscription of processes to processors
	($ppn > $procs_per_node) and next;

	# inner loop iterates of the various run sizes (i.e. number of
	# processors in a parallel job) in the @run_sizes array as
	# defined in cbench.pl
	foreach $numprocs (sort {$a <=> $b} @run_sizes) {
		# check and make sure we don't generate jobs over the max
		# number of procs for the cluster or for the current
		# ppn (as specified in cluster.def)
		($numprocs > $max_ppn_procs{$ppn}) and next;
		($numprocs > $max_procs) and next;

		# honor any max/min processor count arguments from the command line
		(defined $maxprocs_cmdline) and ($numprocs > $maxprocs_cmdline) and next;
		(defined $minprocs_cmdline) and ($numprocs < $minprocs_cmdline) and next;
		(defined $procs_cmdline) and ($numprocs != $procs_cmdline) and next;

		# iterate over the job templates we need to process
		foreach $job (sort keys %templates) {
			# figure out how many nodes we need based on the number of
			# procs and ppn
			$numnodes = calc_num_nodes($numprocs,$ppn);

			# this check should be superflous but do it anyway
			($numnodes > $max_nodes) and next;

			if (!$redundant) {
				# don't generate jobs for redundant 1-node cases, like the following:
				# mpiexec -npernode 8 -np 2 ...
				# mpiexec -npernode 4 -np 2 ...
				($ppn > $numprocs) and next;
			}

			# build the full job name
			$jobname = "$job-".$ppn."ppn-$numprocs";

			# All Cbench jobs are isolated within their own named directory.
			# These dirs are withing the test ident directory and are
			# named for the jobname. This just helps keep job stuff orderly
			# and an added benefit is being able to tuck away extra files as
			# we see fit in the job directories.
			(! -d "$testset_path\/$ident\/$jobname") and mkdir "$testset_path\/$ident\/$jobname",0750;

			# iterate over batch and interactive job generation
			foreach $runtype (('batch','interactive')) {
				my $captype = uc $runtype;

				# prime the output buffers with the appopriate template
				$outbuf = $templates{$job}{$runtype};

				# here we do all the standard substitutions
				$outbuf = std_substitute($outbuf,$numprocs,$ppn,$numnodes,
							$runtype,$default_walltime,$testset,$jobname,$ident,$job);

				# other substitutions
				$outbuf =~ s/TESTDIR_HERE/$testdir/gs;
				$outbuf =~ s/XHPL_BIN_HERE/$xhplbin/gs;
				$outbuf =~ s/HPCC_BIN_HERE/$hpccbin/gs;

				# run any custom generation inner loop work
				my $ret = custom_gen_innerloop($outbuf,$numprocs,$ppn,$numnodes,
							$runtype,$default_walltime,$testset,$jobname,$ident,$job);
				if ($ret) {
					# custom_gen_innerloop returned some sort of error
					# skip the remainder of this loop
					next;
				}

				# build the filename
				($runtype eq 'batch') and $outfile = "$jobname\.pbs";
				($runtype eq 'interactive') and $outfile = "$jobname\.sh";

				($DEBUG) and print "Writing $captype script for $jobname\n";

				# write out the generated job file
				open (OUT,">$testset_path\/$ident\/$jobname\/$outfile") or die
					"Could not write $testset_path\/$ident\/$jobname\/$outfile ($!)";
				print OUT $outbuf;
				close(OUT);

				($runtype eq 'interactive') and
					`/bin/chmod gu+x $testset_path\/$ident\/$jobname\/$outfile`;
			}
		}
	}
}


sub custom_gen_init {
	my $testset = shift;

	debug_print(3,"DEBUG: entering custom_gen_init()\n");

	# loop through the benchmarks/tests/apps found to find any benchmark
	# specific generate stuff to init
	foreach $job (keys %templates) {
		debug_print(3,"DEBUG:custom_gen_init() looking for benchmark \'$job\' customizations\n");
		if (exists $custom_gen_hash{benchmark}{$job}{init}) {
			debug_print(2,"DEBUG:custom_gen_init() calling $custom_gen_hash{benchmark}{$job}{init} \n");
			my $funcname = "$custom_gen_hash{benchmark}{$job}{init}";
			*func = \&$funcname;
			func($testset);	
		}
	}
}


sub custom_gen_innerloop {
	my $outbuf = shift;
	my $numprocs = shift;
	my $ppn = shift;
	my $numnodes = shift;
	my $runtype = uc shift;
	my $walltime = shift;
	my $testset = shift;
	my $jobname = shift;
	my $ident = shift;
	my $job = shift;

	debug_print(3,"DEBUG: entering custom_gen_innerloop()\n");

	# specific generate stuff to run in the inner most generate loop
	if (exists $custom_gen_hash{benchmark}{$job}{innerloop}) {
		debug_print(2,"DEBUG:custom_gen_innerloop() calling $custom_gen_hash{benchmark}{$job}{innerloop} \n");
		my $funcname = "$custom_gen_hash{benchmark}{$job}{innerloop}";
		*func = \&$funcname;
		return func($outbuf,$numprocs,$ppn,$numnodes,
			$runtype,$default_walltime,$testset,$jobname,$ident,$job);
	}
}


sub xhpl_gen_init {
	my $testset = shift;

	debug_print(3,"DEBUG: entering xhpl_gen_init()\n");

	# if we are generating for the shakedown testset, we use low
	# memory utilization setup for Linpack to speed the testing up
	if ($testset == "shakedown") {
		@memory_util_factors = (0.45);
	}

	# read in the xhpl HPL.dat config file generation template
	$file = "$bench_test\/$testset\/xhpl_dat.in";
	open (INFILE,"<$file") or die
		"ERROR:xhpl_gen_init() Could not open $file ($!)";
	undef $/;
	$xhpl_dat = <INFILE>;
	close(INFILE);
	$/ = "\n";
}


sub xhpl_gen_innerloop {
	my $outbuf = shift;
	my $numprocs = shift;
	my $ppn = shift;
	my $numnodes = shift;
	my $runtype = uc shift;
	my $walltime = shift;
	my $testset = shift;
	my $jobname = shift;
	my $ident = shift;
	my $job = shift;

	debug_print(3,"DEBUG: entering xhpl_gen_innerloop()\n");

	# We try generating the HPL.dat file first so that we can
	# abort properly if compute_PQ() fails w/o writing any
	# files.
	#
	# now we need to generate the HPL.dat file for this job. First,
	# we need to figure out the N, P, and Q parameters for this
	# particular job. N is computed based on the total memory available
	# per processor and a memory utlization factor (or array of factors
	# which will generate multiple N values for HPL.dat). P and Q are
	# computed based on the heuristic that HPL "likes" a P:Q ratio
	# of 1:k with k in [1..3].
	my ($P, $Q) = compute_PQ($numprocs);
	if ($P == 0 or $Q == 0) {
		print "No files generated for linpack $jobname\n";
		rmdir "$testset_path\/$ident\/$jobname";
		return 1;
	}
	my @Nvals = compute_N($numprocs,$ppn);
	my $num_Nvals = @Nvals;
	$datbuf = $xhpl_dat;
	$datbuf =~ s/XHPL_NUM_N_HERE/$num_Nvals/gs;
	$datbuf =~ s/XHPL_N_HERE/@Nvals/gs;
	$datbuf =~ s/XHPL_P_HERE/$P/gs;
	$datbuf =~ s/XHPL_Q_HERE/$Q/gs;

	# write out the generated HPL.dat file
	my $full_test_path = "$testset_path\/$ident\/$jobname\/HPL.dat";
	open (OUTFILE,">$full_test_path") or die
		"Could not write $full_test_path ($!)";
	print OUTFILE $datbuf;
	close(OUTFILE);

	return 0;
}


sub usage {
    print "USAGE: $0 \n";
    print "Cbench script to generate jobs in the $testset test set\n".
          "   --ident          Identifying string for the test\n".
		  "   --binident       Binary tree identifier\n".
	#	  "   --binname        Use a non-default binary name\n".
	#	  "                      e.g. --binname xhpl.superoptimized\n".
		  "   --xhplbin        Use a non-default Linpack binary name\n".
		  "                      e.g. --binname xhpl.superoptimized\n".
		  "   --hpccbin        Use a non-default HPCC binary name\n".
          "   --maxprocs       The maximum number of processors to generate\n".
          "                    jobs for\n".
          "   --maxprocs       The minimum number of processors to generate\n".
          "                    jobs for\n".
          "   --procs          Only generate jobs for a single processor count\n".
          "   --runsizes       Comma separated list of run sizes, i.e. processor\n".
          "                    counts, to generate jobs for.  This overrides\n".
          "                    the default array of run sizes declared in cbench.pl\n".
          "   --testdir <path> Path to use for filesystem testing which also determines\n".
          "                    the filesystem to use implicitly\n".
		  "   --jobcbenchtest <path>  Specify an alternate CBENCHTEST path used when\n".
		  "                           generating job scripts. This is useful for\n".
		  "                           generating jobs in one place and being able to use\n".
		  "                           a different path where the jobs are run, like a \n".
		  "                           lightweight initramfs Linux node for instance\n".
		  "   --redundant      Generate redundant ppn/np combinations (not on by default)\n".
		  "   --debug <level>  Turn on debugging at the specified level\n";
}
