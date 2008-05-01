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
BEGIN {
	die "Please define CBENCHOME!\n" if !defined($ENV{CBENCHOME});
}
use lib $ENV{CBENCHOME};
require "cbench.pl";
$CBENCHOME = $BENCH_HOME = $ENV{CBENCHOME};

# enable/disable color support appropriately
detect_color_support();

use Getopt::Long;
use Data::Dumper;
use Term::ANSIColor qw(:constants color colored);
$Term::ANSIColor::AUTORESET = 1;

# pre init some vars before command line parsing
my $xhplbin = 'xhpl';
my $hpccbin = 'hpcc';

my $testset;

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
	'testset=s' => \$testset,
	'debug:i' => \$DEBUG,
	'help' => \$help,
	'redundant' => \$redundant,
	'joblaunch_extraargs=s' => \$joblaunchargs,
	'memory_util_factors|mem_factors=s' => \$new_memory_util_factors,
	'threads|ompthreads|ompnumthreads=i' => \$OMPNUMTHREADS,
    'scaled' => \$scaled,
    'scaled_only' => \$scaled_only,
    'scale_factor=i' => \$scale_factor,
	'gazebo' => \$gazebo,
	'gazebohome|gazhome=s' => \$gazebo_home,
	'gazeboconfig|gazconfig=s' => \$gazebo_config, 
);

if (defined $help) {
	usage();
	exit;
}

if (!defined $testset) {
	$testset = find_testset_identity($0);
}

if (defined $new_memory_util_factors) {
	@memory_util_factors = split(',',$new_memory_util_factors);
	debug_print(1,"DEBUG: overriding memory_util_factors array from command line ".
		"with @memory_util_factors");
}

if (defined $joblaunchargs) {
	# override the cluster.def setting
	debug_print(3,
		"Old joblaunch_extraargs = \"$joblaunch_extraargs\", New joblaunch_extraargs = \"$joblaunchargs\"");
	$joblaunch_extraargs = $joblaunchargs;
}

# Gazebo mode stuff
(defined $gazebo and (!defined $gazebo_home or !defined $gazebo_config)) and
		die "--gazebo requires --gazebohome and --gazeboconfig parameters";
# Gazebo submit config file we'll append to
my $submitconfig_file = "$gazebo_home/submit_configs/$gazebo_config";
# this variable is where we'll incrementally build the Gazebo submit config file
my $submitconfig = "";

$bench_test = get_bench_test();
$testset_path = "$bench_test/$testset";
$DEBUG and print "DEBUG: $bench_test $testset_path\n";

(!defined $ident) and $ident = $cluster_name . "1";
if (!defined $testdir and $testset =~ /^io|shakedown/) {
	$testdir = "$testset_path/$ident/TMP";
	info_print("$0: --testdir not specified, defaulting to $testdir\n");
	mkdir "$testdir",0750;
}


# custom generate processing control hash
%custom_gen_hash = (
	'benchmark' => {
		'npb' => {
			'init' => 'npb_gen_init',
		},
		'xhpl' => {
			'init' => 'xhpl_gen_init',
			'innerloop' => 'xhpl_gen_innerloop',
		},
		'hpcc' => {
			'init' => 'hpcc_gen_init',
			'innerloop' => 'hpcc_gen_innerloop',
		},
		'epetratest' => {
			'innerloop' => 'trilinos_epetratest_gen_innerloop',
		},
	},
	'testset' => {
		'npb' => {
			'joblist' => 'npb_gen_joblist',
			'innerloop' => 'npb_gen_innerloop',
		},
        'lammps' => {
            'init' => 'lammps_gen_init',
            'innerloop' => 'lammps_gen_innerloop',
            'joblist' => 'lammps_gen_joblist',
        },
	},
);

# find and read in the job templates for the testset
my %templates = ();
build_job_templates($testset,\%templates);
# delete the Cbench internal combobatch "job" since we won't use
# it in this context
delete $templates{combobatch};
# by default the list of jobs is the list of templates found
my @job_list = keys %templates;

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

	# Gazebo mode special case
	# we only want to generate jobs for the $procs_per_node ppn case
	if (defined $gazebo) {
		($ppn != $procs_per_node) and next;
	}

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
		foreach $job (custom_gen_joblist($ppn,$numprocs)) {
			# most job templates correspond directly with the name of the job
			my $job_template = $job;
			# NPB is slightly different
			if ($testset eq "npb") {
				$job_template = 'npb';
			}

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

			# Check for Gazebo mode which takes advantage of the gen_jobs smarts
			# but does things completely differently
			if (defined $gazebo) {
				# gazebo doesn't deal with odd process counts so far
				next unless power_of_two($numprocs);

				# gazebo name for the cbench job
				my $gazname =  uc($testset)."-$job";
				# add a line to the Gazebo submit config file
				$submitconfig .= "$gazname $numprocs 50 - $default_walltime *\n";

				# build up the test_exec directory for the job
				(! -d "$gazebo_home\/test_exec\/$gazname") and mkdir "$gazebo_home\/test_exec\/$gazname",0750;
				# symlink the cbench gazebo wrapper
				(! -l "$gazebo_home\/test_exec\/$gazname/run") and 
					system("/bin/ln -s $bench_test/tools/cbench_gazebo_runjob.sh $gazebo_home\/test_exec\/$gazname/run");

				# build the test_exec/$gazname/config file contents
				my $testconfig = "
\$test_config{'AUTHOR'} = \"Cbench\";
\$test_config{'VERSION'} = \"$cbench_version\";
\$test_config{'COMPILER'} = \"\";
\$test_config{'MPILIB'} = \"\";
\$test_config{'JOBSIZE'} = \"\";
\$test_config{'NPES'} = \"2\";
\$test_config{'TIMELIMIT'} = \"$default_walltime\";
\$test_config{'TARGET_WD'} = \"\";
\$test_config{'TEST_PARAMS'} = \"\";
\$test_config{'CMD'} = \"run\";
\@legend = ( );
# The following are non-standard vars that the cbench wrapper uses to figure
# out what to do. We rely on the property of Gazebo that these vars will end
# up in the environment of the running job, and thus we can see them.
\$test_config{'CBENCHTEST'} = \"$bench_test\";
\$test_config{'CBENCH_JOB'} = \"$job\";
\$test_config{'CBENCH_TESTSET'} = \"$testset\";
\$test_config{'CBENCH_TESTIDENT'} = \"$ident\";
";
				# write out the config file
				open (EXECCONFIG,">$gazebo_home\/test_exec\/$gazname/config") or die
					"Could not write $gazebo_home\/test_exec\/$gazname/config ($!)";
				print EXECCONFIG $testconfig;
				close(EXECCONFIG);

				next;
			}

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
				$outbuf = $templates{$job_template}{$runtype};

				# here we do all the standard substitutions
				$outbuf = std_substitute($outbuf,$numprocs,$ppn,$numnodes,
							$runtype,$default_walltime,$testset,$jobname,$ident,$job);

				# other substitutions
				$outbuf =~ s/TESTDIR_HERE/$testdir/gs;
				$outbuf =~ s/XHPL_BIN_HERE/$xhplbin/gs;
				$outbuf =~ s/HPCC_BIN_HERE/$hpccbin/gs;

				# run any custom generation inner loop work, passing a reference to $outbuf so custom
				# innerloops can modify it
				my $ret = custom_gen_innerloop(\$outbuf,$numprocs,$ppn,$numnodes,
							$runtype,$default_walltime,$testset,$jobname,$ident,$job);
				if ($ret) {
					# custom_gen_innerloop returned some sort of error
					# skip the remainder of this loop
					next;
				}

				# build the filename
				($runtype eq 'batch') and $outfile = "$jobname\.$batch_extension";
				($runtype eq 'interactive') and $outfile = "$jobname\.sh";

				debug_print(2,"DEBUG: Writing $captype script for $jobname\n");

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

# Gazebo mode stuff
if (defined $gazebo) {
	open (SUBMITCONFIG,">>$submitconfig_file") or die
		"Could not write $submitconfig_file ($!)";
	print SUBMITCONFIG $submitconfig;
	#print $submitconfig;
	close(SUBMITCONFIG);
}

sub custom_gen_joblist {
	my $ppn = shift;
	my $numprocs = shift;

	# run any specific custom job list subroutine
	if (exists $custom_gen_hash{testset}{$testset}{joblist}) {
		debug_print(2,"DEBUG:custom_gen_joblist() calling $custom_gen_hash{testset}{$testset}{joblist}\n");
		my $funcname = "$custom_gen_hash{testset}{$testset}{joblist}";
		*func = \&$funcname;
		return func($ppn,$numprocs);
	}
	else {
		return @job_list;
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
	my $outbuf = shift;      # a *reference* to the actual $outbuf
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

	# specific generate stuff to run in the inner most generate loop based on testset
	if (exists $custom_gen_hash{testset}{$testset}{innerloop}) {
		debug_print(2,"DEBUG:custom_gen_innerloop() calling $custom_gen_hash{testset}{$testset}{innerloop} \n");
		my $funcname = "$custom_gen_hash{testset}{$testset}{innerloop}";
		*func = \&$funcname;
		return func($outbuf,$numprocs,$ppn,$numnodes,
			$runtype,$default_walltime,$testset,$jobname,$ident,$job);
	}
}

sub npb_gen_init {
	my $testset = shift;

	debug_print(3,"DEBUG: entering npb_gen_init()\n");

	# The list of NAS codes to build. The IS (integer sort) and 
	# EP (embarrasingly parallel) benchmarks are really not workloads
	# we care about.
	my @codes = ('cg','ft','sp','bt','mg','lu');

	# The list of NAS code classes (which correspond to how much memory
	# a NAS code uses and how much work is done) to generate.
	# class A and W are too small for us to really care about
	my @classes = ('B', 'C', 'D');

	@npb_job_list = ();

	foreach my $c (@codes) {
		foreach my $m (@classes) {
			push @npb_job_list, "$c$m";	
		}
	}
	debug_print(3,"DEBUG:npb_gen_init() joblist=".join(',',@npb_job_list));
}

sub npb_gen_joblist {
	my $ppn = shift;
	my $numprocs = shift;

	debug_print(3,"DEBUG: entering npb_gen_joblist($ppn,$numprocs)\n");

	# npb job list will be different depending on the number of processors
	# we are generating for... so we have to cull the list each time
	my @tmplist = ();
	foreach my $job (@npb_job_list) {
		my ($code) = $job =~ /^(\S+)[ABCD]$/;

		# NAS benchmarks only run on a perfect square number of
		# processors or a power of two number of procs (depending
		# on the benchmark).  Filter out all other run sizes.
		if ($code =~ /sp|bt/) {
			# perfect square codes
			next unless perfect_square($numprocs);
		}
		else {
			# power of two codes
			next unless power_of_two($numprocs);
		}
		push @tmplist, $job;
	}

	debug_print(3,"DEBUG:npb_gen_joblist() joblist=".join(',',@tmplist));
	return @tmplist;
}

sub npb_gen_innerloop {
	my $outbuf = shift;      # a *reference* to the actual $outbuf
	my $numprocs = shift;
	my $ppn = shift;
	my $numnodes = shift;
	my $runtype = uc shift;
	my $walltime = shift;
	my $testset = shift;
	my $jobname = shift;
	my $ident = shift;
	my $job = shift;

	debug_print(3,"DEBUG: entering npb_gen_innerloop($job)\n");

	my ($code,$class) = $job =~ /^(\S+)([ABCD])$/;
	my $npbname = "$code\.$class\.$numprocs";
	debug_print(3,"DEBUG:npb_gen_innerloop() $npbname\n");
	$$outbuf =~ s/NPB_HERE/$npbname/gs;

	return 0;
}

sub hpcc_gen_init {
	my $testset = shift;

	debug_print(3,"DEBUG: entering hpcc_gen_init()\n");

	# if we are generating for the shakedown testset, we use low
	# memory utilization setup for Linpack to speed the testing up
	if ($testset eq "shakedown") {
		@memory_util_factors = (0.45);
	}

	# read in the hpcc hpccinf.txt config file generation template
	$file = "$bench_test\/$testset\/hpccinf_txt.in";
	open (INFILE,"<$file") or die
		"ERROR:hpcc_gen_init() Could not open $file ($!)";
	undef $/;
	$hpccinf_txt = <INFILE>;
	close(INFILE);
	$/ = "\n";
}

sub hpcc_gen_innerloop {
	my $outbuf = shift;      # a *reference* to the actual $outbuf
	my $numprocs = shift;
	my $ppn = shift;
	my $numnodes = shift;
	my $runtype = uc shift;
	my $walltime = shift;
	my $testset = shift;
	my $jobname = shift;
	my $ident = shift;
	my $job = shift;

	debug_print(3,"DEBUG: entering hpcc_gen_innerloop()\n");

	# We try generating the hpccinf.txt file first so that we can
	# abort properly if compute_PQ() fails w/o writing any
	# files.
	#
	# now we need to generate the hpccinf.txt file for this job. First,
	# we need to figure out the N, P, and Q parameters for this
	# particular job. N is computed based on the total memory available
	# per processor and a memory utlization factor (or array of factors
	# which will generate multiple N values for HPL.dat). P and Q are
	# computed based on the heuristic that HPL "likes" a P:Q ratio
	# of 1:k with k in [1..3].
	my ($P, $Q) = compute_PQ($numprocs);
	if ($P == 0 or $Q == 0) {
		warning_print("No files generated for hpcc $jobname\n");
		rmdir "$testset_path\/$ident\/$jobname";
		return 1;
	}
	my @Nvals = compute_N($numprocs,$ppn);
	my $num_Nvals = @Nvals;
	$datbuf = $hpccinf_txt;
	$datbuf =~ s/HPCC_NUM_N_HERE/$num_Nvals/gs;
	$datbuf =~ s/HPCC_N_HERE/@Nvals/gs;
	$datbuf =~ s/HPCC_P_HERE/$P/gs;
	$datbuf =~ s/HPCC_Q_HERE/$Q/gs;

	# write out the generated hpccinf.txt file
	my $full_test_path = "$testset_path\/$ident\/$jobname\/hpccinf.txt";
	open (OUTFILE,">$full_test_path") or die
		"Could not write $full_test_path ($!)";
	print OUTFILE $datbuf;
	close(OUTFILE);

	return 0;
}

sub xhpl_gen_init {
	my $testset = shift;

	debug_print(3,"DEBUG: entering xhpl_gen_init()\n");

	# if we are generating for the shakedown testset, we use low
	# memory utilization setup for Linpack to speed the testing up
	if ($testset eq "shakedown") {
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
	my $outbuf = shift;      # a *reference* to the actual $outbuf
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
		warning_print("No files generated for linpack $jobname\n");
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


sub trilinos_epetratest_gen_innerloop {
	my $outbuf = shift;      # a *reference* to the actual $outbuf
	my $numprocs = shift;
	my $ppn = shift;
	my $numnodes = shift;
	my $runtype = uc shift;
	my $walltime = shift;
	my $testset = shift;
	my $jobname = shift;
	my $ident = shift;
	my $job = shift;

	debug_print(3,"DEBUG: entering trilinos_epetratest_gen_innerloop()\n");

	# The epetra test program wants NumProcX and NumProcY parameters on the command line,
	# with NumProcX * NumProcY == NP.  The examples on http://www.sandia.gov/benchmarks show
	# that the factors should look like what HPL and HPCC use for P and Q values, so the
	# compute_PQ() function can be used.
	my ($X, $Y) = compute_PQ($numprocs);
	if ($X == 0 or $Y == 0) {
		print "Couldn't determine NumProcX and NumProcY for NP=$numprocs.  Skipping $jobname...\n";
		rmdir "$testset_path\/$ident\/$jobname";
		return 1;
	}

	# Epetra prefers X >= Y though
	my $NUMPROCS_X = max($X,$Y);
	my $NUMPROCS_Y = min($X,$Y);
	$$outbuf =~ s/NUMPROCS_X_HERE/$NUMPROCS_X/gs;
	$$outbuf =~ s/NUMPROCS_Y_HERE/$NUMPROCS_Y/gs;

	return 0;
}

sub lammps_gen_init {
	# no-op for now
}

sub lammps_gen_joblist {
    my $ppn = shift;
    my $numprocs = shift;

    my @scaled_joblist = qw(rhodo.scaled chain.scaled lj.scaled);
    my @normal_joblist = qw(rhodo chain lj); #this is only a temporary list; LAMMPS has many more codes use 
    debug_print(3, "DEBUG: entering lammps_gen_joblist($ppn,$numprocs)\n");

    #we only want scaled jobs to be created when specified by the user

    my @tmplist =();

    if ( not defined $scaled and not defined $scaled_only ) {
        debug_print(3, "DEBUG: generating only lammps normal jobs");
        push(@tmplist, @normal_joblist);
    }

    #create only scaled jobs when --scaled-only parameter is set
    #create scaled jobs with normal jobs when --scaled parameter is set
    else {
        debug_print(3, "DEBUG: generating lammps scaled jobs");
        #add scaled jobs 
            push(@tmplist, @scaled_joblist);

        if ( not defined $scaled_only ) {
            #also generate normal jobs for --scaled parameter
            debug_print(3, "DEBUG: generating lammps normal jobs");
            push(@tmplist, @normal_joblist );
        }
    }

    debug_print(3, "DEBUG: lammps_gen_joblist() joblist=".join(',',@tmplist));
    return @tmplist; 
}   

sub lammps_gen_innerloop {
    my $outbuf = shift;      # a *reference* to the actual $outbuf
	my $numprocs = shift;
    my $ppn = shift;
    my $numnodes = shift;
    my $runtype = uc shift;
    my $walltime = shift;
    my $testset = shift;
    my $jobname = shift;
    my $ident = shift;
    my $job = shift;

    debug_print(3,"DEBUG: entering custom_lammps_innerloop()\n");

    debug_print(2,"DEBUG:lammps_gen_innerloop() populating files in: $testset_path\/$ident\/$jobname\n");

	# since the job templates point to the input deck with a path, we don't seem to need
	# to copy it into the jobs working directory
	#
	#copy modified lammps in.* files to each jobdir
    #$jobname !~ /scaled/ and lammps_copy_files("$testset_path\/$ident\/$jobname", $job);

	# symlink any data files into the jobs directory from the directory holding all the input
	# decks and data files that only need to be read
	#
	# this way we don't have to muck with the input decks path to the data files, they just
	# look in the CWD
	my $jobbase = $job;
	$jobbase =~ s/\.scaled//;
	if (-f "$testset_path\/bench\/data.$jobbase") { 
		debug_print(2,"DEBUG:lammps_gen_innerloop() symlinking $testset_path\/bench\/data.$jobbase\n");
		system("/bin/ln -sf $testset_path/bench/data.$jobbase $testset_path\/$ident\/$jobname\/data.$jobbase");
	}

	#set up the scaling parameters based on this jobsize
    if ( $jobname =~ /scaled/ ) {
		my $scale = 1;
		(defined $scale_factor) and $scale = $scale_factor;

		# according to LAMMPS readme file, LJ and EAM decks need to have Px,Py,Pz a factor
		# of 20 greater than other decks
		($job =~ /lj\.scaled|eam.scaled/) and $scale = 20 * $scale;

		my $factors = "$scale $scale $scale";
        my $scaling_params = lammps_get_scaling_params($numprocs, $factors);
		debug_print(2,"DEBUG:lammps_gen_innerloop($job) scaling_params=$scaling_params");
        $$outbuf =~ s/SCALING_PARAMS_HERE/$scaling_params/gs;
    }
	else {
        $$outbuf =~ s/SCALING_PARAMS_HERE//gs;
	}


    return 0;
}

sub usage {
    print "USAGE: $0 \n";
    print "Cbench script to generate jobs in the $testset test set\n".
          "   --testset <name> Override the name of the testset (optional).\n".
          "   --ident          Identifying string for the test\n".
		  "   --binident       Binary tree identifier\n".
	#	  "   --binname        Use a non-default binary name\n".
	#	  "                      e.g. --binname xhpl.superoptimized\n".
		  "   --xhplbin        Use a non-default Linpack binary name\n".
		  "                      e.g. --binname xhpl.superoptimized\n".
		  "   --hpccbin        Use a non-default HPCC binary name\n".
          "   --maxprocs       The maximum number of processors to generate\n".
          "                    jobs for\n".
          "   --minprocs       The minimum number of processors to generate\n".
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
		  "   --joblaunch_extraargs <args>  Override the joblaunch_extraargs setting in cluster.def\n".
		  "   --memory_util_factors  Override the cluster.def \@memory_util_factors array.\n".
		  "                          For example:\n".
		  "                            --memory_util_factors 0.10,0.77,0.85\n".  
		  "   --ompthreads <num> \n".
		  "   --ompnumthreads <num> \n".
		  "   --threads <num>  Tell Cbench to use the specified number of threads per\n".
		  "                    process where applicable. This is usually equivalent to\n".
		  "                    setting OMP_NUM_THREADS environment variable\n".
		  "   --gazebo               Gazebo mode of operation\n".
		  "   --gazebohome <path>    Where the Gazebo tree is located\n".
		  "   --gazeboconfig <name>  Name of the Gazebo submit_config that will be APPENDED to\n".
          "   --debug <level>  Turn on debugging at the specified level\n";
	print "   \nLAMMPS scaling options:\n".
          "   --scaled                  Generate scaled jobs along with normal jobs\n".
          "   --scaled_only             Generate scaled jobs only\n".
          "   --scale_factor <factor>   The additional factor by which you would like to \n".
          "                             scale the x,y,z values in the scaling benchmarks\n".
		  "                             For example:\n".
		  "                               --scaled_factor 20\n";
}
