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

# First pass at a script purposed to deal with running a standard repeatable
# single node benchmarking suite, captializing on other Cbench capabilities
# and building a report about it.
#
# This first pass won't address, in the interest of time and needing to see
# how things develop, Marcus's monte carlo multi-node data gathering scenario...

# need to know where everything cbench lives!
BEGIN {
	die "Please define CBENCHOME!\n" if !defined($ENV{CBENCHOME});
}
use lib $ENV{CBENCHOME};
require "cbench.pl";
$CBENCHOME = $BENCH_HOME = $ENV{CBENCHOME};

# add Cbench perl library to the Perl search path
use lib "$ENV{CBENCHOME}\/perllib";

# enable/disable color support appropriately
detect_color_support();

use Getopt::Long;
use Data::Dumper;
use Statistics::Descriptive;
use File::stat;
use Term::ANSIColor qw(:constants color);
$Term::ANSIColor::AUTORESET = 1;


GetOptions(
	'numcore|numcpu=i' => \$NUMCPUS,
	'ident=s' => \$ident,
	'binident=s' => \$binident,
	'destdir=s' => \$destdir,
	'run' => \$run,
	'report' => \$report,
	'savepdfsrc' => \$savepdfsrc,
	'pdftag=s' => \$pdftag,
	'node=s' => \$node,
	'tests=s' => \$tests,
	'dryrun|dry-run' => \$dryrun,
	'debug=i' => \$DEBUG,
        'help' => \$help,
        'nodetonode' => \$nodetonode,
        'coretonode' => \$coretonode,
);

(defined $help) and usage() and exit;

(!defined $tests) and $tests = 'stream|cachebench|dgemm|mpistreams|linpack|npb';

(!defined $run and !defined $report) and ($run = 1 and $report = 0);

(!defined $ident) and $ident = $cluster_name . "1";
(!defined $destdir) and $destdir = ".";
# make sure the test ident directory is there with the proper test identification
(! -d "$destdir\/$ident") and mkdir "$destdir\/$ident",0750;

# find the Cbench testing tree
my $bench_test = get_bench_test();
my $bindir = 'bin';
my $binidentopt = '';
(defined $binident) and $bindir = "bin.$binident";
(defined $binident) and $binidentopt = "--binident $binident";
my $binpath = "$bench_test/$bindir";
(defined $DEBUG) and print "DEBUG: test tree path = $bench_test\n";

my $hn = `/bin/hostname`;
chomp $hn;
(defined $report and defined $node) and $hn = $node;

# When we use other Cbench testsets we will create unique test identifiers
# in each testset for whatever we are doing. This is the basename for those
# test identifiers.
my $identbase = "snb_$ident";

# When we are generating a report we'll build sections in the Latex doc
# as we go along. This buffer captures all that will be added in the core
# of the doc
my @report_core_buf= ();

# log file for the run
my $logfile = "$destdir/snb.$hn.$ident.log";
open (LOG,">>$logfile") or die
	"Could not open $logfile ($!)";

# probe the number of cpu cores
my $numcores = linux_num_cpus();
# the number of cores read in from cpuinfo files generated during a run
my $num_cores_counted = 0;

# become the leader of a new process group so that all the child test
# processes will see our signals
setpgrp(0,0);
$SIG{CHLD} = \&REAPER;
$SIG{INT} = \&CATCH;
$SIG{TERM} = \&CATCH;
$SIG{KILL} = \&CATCH;
$SIG{USR1} = \&CATCH;
$SIG{USR2} = \&CATCH;


(defined $run) and
	logmsg("INITIATING Single Node Benchmarking RUN on node $hn, test identifier is $ident, test regex: $tests");
(defined $report and !defined $run) and
	logmsg("INITIATING Single Node Benchmarking REPORT for node $hn, test identifier is $ident");

# grab some info on the node
if (defined $run) {
	runcmd("/bin/uname -s -r -m -p -i -o","uname","overwrite");
	runcmd("cat /proc/cpuinfo","cpuinfo","overwrite");
	runcmd("cat /proc/meminfo","meminfo","overwrite");

        # hwloc info
        runcmd("$binpath/lstopo","hwloc","file");
}
if ($report) {
	add_section("Basic Node Description");

	my $text = "";
	my $num_physical_cpus = 0;
	my $num_logical_cpus = 0;

	# uname info
	my $uname = `cat $destdir/$ident/$hn.snb.uname.out`;
	$text .= "$uname";

	# cpu/core info
	my @cpudata = `cat $destdir/$ident/$hn.snb.cpuinfo.out`;
        my %cpumap = linux_parse_cpuinfo(\@cpudata);
	(defined $DEBUG) and print Dumper(%cpumap);
        # decode what the cpumap hash tells us
        foreach (keys %cpumap) {
                    (/model/) and next;
                    $num_physical_cpus++;
            $num_cores_counted += $cpumap{$_}{'cores'};
            $num_logical_cpus += scalar @{$cpumap{$_}{'logical'}};
        }
	# if no detailed core/socket info was found, fall back to just
	# simple logical cpu count 
	if (exists $cpumap{'COUNT'}) {
		$num_physical_cpus = $num_cores_counted = $cpumap{'COUNT'};
	}


	$text .= "Number of Physical Processors: $num_physical_cpus\n".
			"Number of Processing Cores: $num_cores_counted\n";

	# memory info
	my @meminfo = `cat $destdir/$ident/$hn.snb.meminfo.out`;
	foreach (@meminfo) {
		if (/MemTotal:\s+(\d+)\s+kB/) {
			my $mb = int ($1 / 1024);
			$text .= "Total Memory: $mb MB\n";
		}
	}

	add_text_raw($text);
	# more processor info
	add_list("Processor Models:",\@{$cpumap{'model'}});

}

#
# streams, via node_hw_test
if ($tests =~ /stream/ and $run) {
	logmsg("Starting STREAMS testing");
	runcmd("$bench_test/nodehwtest/node_hw_test --ident $identbase $binidentopt --match streams --debug 1","streams","overwrite");
}
# parse and report on streams
if ($report) {
	runcmd("$bench_test/nodehwtest/nodehwtest_output_parse.pl --ident $identbase --noerrors | /bin/grep streams","streams_data","overwrite");

	my $out = "$destdir/$ident/$hn.snb.streams_data.out";
	open (IN,"<$out") or do {
		print "Could not open $out ($!)\n";
	};
	my @rawdata = <IN>;
	close(IN);

	# digest the raw data
	my %data = ();
	my $text = "";
	foreach my $l (@rawdata) {
		chomp $l;
		# streams_add: mean=3888.0000 max=3888.0000 min=3888.0000 stddev=0.0000  (sample count=1)
		if ($l =~ /streams\_(\w+)\:\s+mean=(\d+\.\d+)\s+max=(\d+\.\d+)\s+min=(\d+\.\d+)\s+stddev=(\d+\.\d+)\s+/)
		{
			$data{$1}{'mean'} = $2;
			$data{$1}{'max'} = $3;
			$data{$1}{'min'} = $4;
			$text .= "streams $1: $3 MB/s\n" unless ($1 =~ /failed/);
		}
	}
	#print Dumper (%data);

	add_section("STREAM Results");
	add_text_raw($text);
	add_text("\\clearpage\n");
}

#
# stream2
if ($tests =~ /streams/ and $run) {
	logmsg("Starting STREAM2 testing");
	# run any stream2 binary
	my @binlist = `cd $binpath/hwtests;ls -1 stream2-* 2>/dev/null`;
	foreach my $bin (@binlist) {
		chomp $bin;
		runcmd("$binpath/hwtests/$bin",$bin,"overwrite");
	}
}
# parse and report on stream2
if ($report) {
	# could be multiple stream2 binaries run...
	my %data = ();
	foreach my $out (`cd $destdir/$ident;ls -1 $hn.snb.stream2*.out`) {
		chomp $out;
		my $file = "$destdir/$ident/$out";
		open (IN,"<$file") or do {
			"print Could not open $file ($!)\n";
		};
		my @rawdata = <IN>;
		close(IN);

		# find the stream2 binary name
		(my $bin) = $out =~ /$hn\.snb\.(\S+)\.out/;

		# grab the raw stream2 results
		for my $l (@rawdata) {
			if ($l =~ /\s+(\d+)\s+\d+\s+(\d+\.\d+)\s+(\d+\.\d+)\s+(\d+\.\d+)\s+(\d+\.\d+)\s+(\d+\.\d+)\s+/)
			{
				$data{$1}{"$bin-fill"} = $2;
				$data{$1}{"$bin-copy"} = $3;
				$data{$1}{"$bin-daxpy"} = $4;
				$data{$1}{"$bin-sum"} = $5;
			}
		}
	}
	#print Dumper (%data);

	build_gnuplot_graph("stream2","STREAM2 Results","Bytes","MB/s",\%data,["set log x"]);
	add_section("STREAM2 Results");
	add_figure("stream2.pdf","STREAM2 Data","stream2");
}

#
# cachebench, via node_hw_test
if ($tests =~ /cachebench/ and $run) {
	logmsg("Starting CACHEBENCH testing");
	runcmd("$bench_test/nodehwtest/node_hw_test --ident $identbase $binidentopt --match cachebench --debug 1 --dump","cachebench","overwrite");
}
# parse and report on cachebench
if ($report) {
	my $out = "$destdir/$ident/$hn.snb.cachebench.out";
	open (IN,"<$out") or do {
		"print Could not open $out ($!)\n";
	};
	my @rawdata = <IN>;
	close(IN);

	# digest the raw data
	my %data = ();
	my $test_context = "NONE";
	foreach my $l (@rawdata) {
		if ($l =~ /====> (\S+)/) {
			$test_context = $1;
		}
		elsif ($l =~ /^(\d+)\s+(\d+\.\d+)/) {
			push @{$data{$1}{$test_context}}, $2;
		}
	}
	#print Dumper (%data);

	# average the results at each vector lenght and put into a hash data
	# structure suitable for passing to the gnuplot generate
	my %plotdata = ();
	foreach my $len (sort {$a <=> $b} keys(%data)) {
		foreach my $tst (sort {$a cmp $b} keys(%{$data{$len}})) {
			my $statvar = Statistics::Descriptive::Full->new();
			$statvar->add_data(@{$data{$len}{$tst}});
			$plotdata{$len}{$tst} = $statvar->mean();
		}
	}
	#print Dumper (%plotdata);

	build_gnuplot_graph("cachebench","Cachebench Results","Bytes","MB/s",\%plotdata,["set log x"]);
	add_section("Cachebench Results");
	add_figure("cachebench.pdf","Cachebench Data","cachebench");
}

#
# dgemm memsize vs gflops, nodeperf2
if ($tests =~ /dgemm/ and $run) {
    
	logmsg("Starting DGEMM (nodeperf2) testing");

	# reset the output file
	runcmd("/bin/true","nodeperf2","overwrite");
	# loop through dgemm matrix sizes
	my $x = 20000;
	for (my $n = 2; $n <= 2048; )  {
		# figure out the number of iterations we want to do, rule of thumb
		my $i = int ($x / $n);
		(($i % 2) == 1) and $i++;
		# run the dgemm test multiple times so we can average
		for my $j (1..3) {
                        $ENV{OMP_NUM_THREADS}=$numcores; # the exec("exec $cmd") command doesn't like 'export', so
                                                         # set the environment variable here and hope it gets
                                                         # inherited by the nodeperf2 process - RKB
			runcmd("$binpath/nodeperf2-nompi -i $i -s $n","nodeperf2");
                        #runcmd("export OMP_NUM_THREADS=$numcores; $binpath/nodeperf2-nompi -i $i -s $n","nodeperf2");
		}
		$n = int ($n * 1.5);
	}
}
# parse and report on dgemm
if ($report) {
	my $out = "$destdir/$ident/$hn.snb.nodeperf2.out";
	open (IN,"<$out") or do {
		print "Could not open $out ($!)\n";
	};
	my @rawdata = <IN>;
	close(IN);

	# grab the raw dgemm results
	my %data = ();
	for my $l (@rawdata) {
		if ($l =~ /\s+\S+: NN lda=\d+ ldb=\s*\d+ ldc=\d+ \d+ \d+ \d+ (\d+\.\d+) mem=(\d+) MB\s+/) {
			push @{$data{$2}}, $1;
		}
	}

	# average the dgemm results and put into a hash data structure suitable for
	# passing to the gnuplot generate
	my %plotdata = ();
	foreach my $mem (sort {$a <=> $b} keys(%data)) {
		my $statvar = Statistics::Descriptive::Full->new();
		$statvar->add_data(@{$data{$mem}});
		$plotdata{$mem}{'dgemm'} = $statvar->mean();
	}
	#print Dumper (%plotdata);

	build_gnuplot_graph("nodeperf2","DGEMM Results","Megabytes","Megaflops",\%plotdata);
	add_section("DGEMM Results");
	add_text("All DGEMM tests a run using a single process with OMP number of threads set".
		" to the maximum number of computing cores on the node\n");
	add_figure("nodeperf2.pdf","DGEMM Data","nodeperf2");
}

#
# mpi streams
if ($tests =~ /mpistreams/ and $run) {
	logmsg("Starting Multiprocess STREAMS (mpi streams) testing");
	# reset the output file
	runcmd("/bin/true","mpistreams","overwrite");
	
	# need this for building mpi job launches
	my $funcname = "$joblaunch_method\_joblaunch_cmdbuild";
	*func = \&$funcname;

	for my $np (1..$numcores) {
		system("echo '====> $np processes' >> $destdir/$ident/$hn.snb.mpistreams.out");
		my $jobcmd = func($np,$procs_per_node,1);
		$jobcmd .= "$binpath/hwtests/stream-mpi";
		runcmd("$jobcmd","mpistreams");
	}

}
# parse and report on mpi streams
if ($report) {
	my $out = "$destdir/$ident/$hn.snb.mpistreams.out";
	open (IN,"<$out") or do {
		print "Could not open $out ($!)\n";
	};
	my @rawdata = <IN>;
	close(IN);

	# digest the raw data
	my %data = ();
	my $np = "NONE";
	foreach my $l (@rawdata) {
		if ($l =~ /====> (\d+) processes/) {
			$np = $1;
		}
		elsif ($l =~ /^(\w+):\s+(\d+\.\d+)\s+(\d+\.\d+)\s+(\d+\.\d+)\s+(\d+\.\d+)/) {
			#Copy:       6049.9450      0.1059      0.1058      0.1061
			#Scale:      5980.1428      0.1072      0.1070      0.1073
			#Add:        6998.5623      0.1373      0.1372      0.1376
			#Triad:      5903.7286      0.1628      0.1626      0.1630
			$data{$np}{$1} = $2;
		}
	}  
	#print Dumper (%data);

	build_gnuplot_graph("mpistreams","Multiple Process STREAMS Results (MPI Streams)",
		"Number of Processes","MB/s",\%data);
	add_section("Multiple Process STREAMS Results (MPI Streams)");
	add_figure("mpistreams.pdf","MPISTREAMS Data","mpistreams");
}


# xhpl, use testset
if ($tests =~ /linpack/ and $run) {
	logmsg("Starting Linpack testing");

	# record the memory utilization factors from cluster.def for the run
	my $tmptxt = "MEM_UTIL_FACTORS: ". join(',',@memory_util_factors);
	system("echo '$tmptxt' > $destdir/$ident/$hn.snb.linpack.out");
	
	#
	# first we need to generate all the linpack jobs scripts
	logmsg("Generating Linpack testing scripts");

	# do the actual job script generation
	for my $threads (1..$numcores) {
		my $count = $numcores / $threads;
		for my $processes (1..$count) {
			#(($processes * $threads) > $numcores) and next;
			(!power_of_two($processes) and ($processes > 1)) and next;
			(!power_of_two($threads) and ($threads > 1)) and next;

			my $pq = $threads * $processes;
			my $cmd = "$bench_test/linpack/linpack_gen_jobs.pl --ident ".$identbase."_".
				$threads."threads --runsizes $processes $binidentopt --threads $threads";
			print "$cmd\n";
			runcmd("$cmd",'linpack');
		}
	}

	#
	# next we need to run all the linpack jobs scripts that make sense
	logmsg("Running Linpack testing scripts");
	for my $threads (1..$numcores) {
		my $count = $numcores / $threads;
		for my $processes (1..$count) {
			#(($processes * $threads) > $numcores) and next;
			(!power_of_two($processes) and ($processes > 1)) and next;
			(!power_of_two($threads) and ($threads > 1)) and next;

			my $cmd = "$bench_test/linpack/linpack_start_jobs.pl --ident ".$identbase."_".
				$threads."threads --procs $processes --interactive --match $processes\ppn";
			#print "$cmd\n";
			runcmd("$cmd",'linpack');
		}
	}
	
}
# parse and report on linpack
if ($report) {
	runcmd("$bench_test/linpack/linpack_output_parse.pl --match \'$identbase\' --metric gflops --grep",
		"linpack_data","overwrite");

	my $out = "$destdir/$ident/$hn.snb.linpack_data.out";
	open (IN,"<$out") or do {
		print "Could not open $out ($!)\n";
	};
	my @rawdata = <IN>;
	close(IN);

	# grok the grepable data spit out by the linpack output parser and categorize
	# it by number of procs and number of threads
	my %data = ();
	foreach (@rawdata) {
		# SERIES, NUMPROCS, MEAN, MAX, MIN, STDDEVIATION, SAMPLECOUNT, UNITS
		# snb_bl460c-quad-04_4threads-xhpl-2ppn-gflops, 2, 18.1125, 20.1500, 15.1000, 2.2187, 4, GigaFlops
		if (/(\S+)\_(\d+)threads\-(\S+)\-(\d+)ppn\-(\S+),\s+(\d+),\s+(\d+\.\d+),\s+(\d+\.\d+),\s+(\d+\.\d+),\s+(\d+\.\d+),\s+(\d+),\s+(\S+)/) {
			#print "$1 $2 $3 $4 $5 $6 $7 $8 $9 $10 $11 $12\n";
			my $ident = $1;
			my $nthreads = $2;
			my $bench= $3;
			my $nppn = $4;
			my $metric = $5;
			my $nproc = $6;
			my $mean = $7;
			my $max =  $8;
			my $min = $9;
			my $stddev = $10;
			my $count = $11;
			my $units = $12;

			$data{$nproc}{$nthreads} = $8;
		}

	}
	(defined $DEBUG) and print Dumper (%data);

	my $out = "$destdir/$ident/$hn.snb.linpack.out";
	my @rawdata = `cat $out`;
	my $memfactors = 'unknown';
	foreach (@rawdata) {
		if (/MEM_UTIL_FACTORS:\s+(\S+)/) {
			$memfactors = $1;
		}
	}

	build_gnuplot_histogram("linpack","Single Node Linpack Results",
		"Number of MPI Processes","GigaFlops",\%data);
	add_section("Single Node Linpack Results");
	add_text("Memory Utilization Factors Used: $memfactors\n\n");
	add_figure("linpack.pdf","Linpack Data","linpack");

	# now, normalize the data by number of computing threads and build another
	# graph (easier to see scaling efficiency)
	for my $np (keys %data) {
		for my $nt (keys %{$data{$np}}) {
			my $tmp = $data{$np}{$nt};
			$data{$np}{$nt} = $tmp / ($np * $nt);
		}
	}
	build_gnuplot_histogram("linpack-normalized","Normalized Single Node Linpack Results",
		"Number of MPI Processes","GigaFlops / Total Number of Computing Threads",\%data);
	add_text("This graph normalizes the Single Node Linpack data by the total number of ".
		" computing threads so that scaling efficiency can more easily be seen.  The".
		" total number of".
		" computing threads is product of the number of MPI processes".
		" and the number of BLAS threads.\n");
	add_figure("linpack-normalized.pdf","Normalized Linpack Data","linpack-normalized");
}


# NPB, use testset
if ($tests =~ /npb/ and $run) {
	logmsg("Starting NAS Parallel Benchmark testing");
	# reset the output file

	runcmd("/bin/true","npb","overwrite");

	# generate the test identifier for our npb testing runs
	my $cmd = "$bench_test/npb/npb_gen_jobs.pl --ident ".$identbase.
		" --maxprocs $numcores $binidentopt --redundant";
	runcmd("$cmd",'npb');

	# do our npb testing runs starting with the biggest memory runs first,
	# i.e. the C class binaries
	my $cmd = "$bench_test/npb/npb_start_jobs.pl --ident $identbase --minprocs 1".
		" --maxprocs $numcores --match \'C-".$numcores."ppn\' --interactive";
	runcmd("$cmd",'npb');
}
# parse and report on NPB
if ($report) {
	add_section("Single Node NAS Parallel Benchmark Results");

	#--xrange 0,4 --norm numprocs --match 'snb_bl465c-dual-05' --collapse 'snb'

	# build the first NPB plot
	runcmd("/bin/rm -f $bench_test/npb/npb.ps","npb");
	runcmd("$bench_test/npb/npb_output_parse.pl --match \'$identbase\' ".
		"--collapse \'snb\' --xrange 1,$num_cores_counted --yrange 0,\* --gnuplot ".
		"--linewidth 4 </dev/null","npb");
	runcmd("/bin/cp -f $bench_test/npb/npb.ps $destdir/.","npb");
	ps2pdf("$destdir/npb.ps");
	add_figure("npb.pdf","NAS Parallel Benchmark Data","npb");

	# build the second NPB plot which is normalized to show scaling efficiency
	runcmd("/bin/rm -f $bench_test/npb/npb.ps","npb");
	runcmd("$bench_test/npb/npb_output_parse.pl --match \'$identbase\' --scaledparalleleff ".
		"--collapse \'snb\' --xrange 1,$num_cores_counted --gnuplot --yrange 0,\* ".
		"--linewidth 4 </dev/null","npb");
	runcmd("/bin/cp -f $bench_test/npb/npb.ps $destdir/npb-normalized.ps","npb");
	ps2pdf("$destdir/npb-normalized.ps");
	add_text("This graph normalizes the NAS Parallel Benchmark data so that scaled parallel".
		"efficiency can be seen. A flat line at a Y value of 1 would be 100% efficiency.\n");
	add_figure("npb-normalized.pdf","Normalized NAS Parallel Benchmark Data","npb-normalized");
}


# hpcc, use testset, need cbench tweaked version with cmdline options

#
# test out how locking down threads to various core combinations
# affects performance. this kind of tests linux scheduler prowess
# as well as other core locality things. the data is quite open for
# interpretation...


# NUMA Memory-Access Tests
if ($tests =~ /numa-mem/ and $run) {
    my $logstr = "Starting NUMA Memory-Access ";
    my $mode = "unknown";
    $logstr .= $mode = ($coretonode) ? "core_to_node" : "node_to_node";
    $logstr .= " Testing";
    logmsg($logstr);


    # to test memory-access characteristics, we look at memory read latency and memory bandwidth
    for my $metric ("latency", "bandwidth") {
        logmsg("Running NUMA $metric tests");

        my $cmdtag = "numa-$metric";
        my $command = "none";
        my @binlist = ();

        # compile an array of binaries for this metric
        if ($metric eq "latency") {
            $command = "$binpath/hwtests/lat_mem_rd";
            @binlist = "lat_mem_rd";
        }
        elsif ($metric eq "bandwidth") {
            # run any stream binary
            @binlist = `cd $binpath/hwtests;ls -1 stream*-* 2>/dev/null`;
        }
        else {
            print STDERR "numa-mem metric $metric is invalid\n";
            next;
        }

        # run the numa tests for each binary in the list
        for my $bin (@binlist) {
            ($main::INTsignalled) and last;
            ($bin =~ /mpi/) and next; # we'll deal with mpi multi-threaded tests later
            chomp $bin;
            $command = "$binpath/hwtests/$bin";

            # verify that the requested executable exists
            $ret = `which $command`;
            if ($ret =~ /^which: no/ or $ret =~ /^$/) {
                print STDERR "run_numa_tests(): can't find $command\n";
            }
            # found the executable - let's run the tests!
            else {

                # need to set up the tests so that they save their output appropriately
                #$command .= " >> $destdir/$ident/$hn.snb.$cmdtag.$date.out 2>&1";
                my $filename = "$destdir/$ident/$hn.snb.$cmdtag.out";

                # default to node-to-node mode
                my $mode = ($coretonode) ? "core-to-node" : "node-to-node";

                # lat_mem_rd needs some additional parameters
                # the stride parameter here probably isn't ideal - perhaps do a sort of stride test on new machines?
                ($command =~ /lat_mem_rd/) and $command .= " -P 1 -S 1000 131072";

                # some Cbench-built STREAM tests can use a lot of memory - do a quick check 
                # before trying to run one of the '10G' or 'big' STREAM tests
                my $free = `free | grep Mem | awk \'{print \$2}\' 2>&1`;
                chomp($free);
                if (($bin =~ /big/ and $free < 2**20) or ($bin =~ /10G/ and $free < 10* 2**30)) {
                    logmsg("  NOT Starting $command: Only " . sprintf("%.2f", $free/(2**20)) . " GiB of memory in the system");
                    next;
                }

                # run the command now
                logmsg("  Starting $command >> $filename");
                run_numa_tests("", $command, $mode, $filename);
            }
        }

    }

    # will need to deal with this stuff to get the multi-threaded stream results
#    # need this for building mpi job launches
#    my $funcname = "$joblaunch_method\_joblaunch_cmdbuild";
#    *func = \&$funcname;
#
#    for my $np (1..$numcores) {
#        system("echo '====> $np processes' >> $destdir/$ident/$hn.snb.mpistreams.out");
#        my $jobcmd = func($np,$procs_per_node,1);
#        $jobcmd .= "$binpath/hwtests/stream-mpi";
#        runcmd("$jobcmd","mpistreams");
#    }

}
# Parse and report on the NUMA tests
if ($report) {
    #runcmd("$bench_test/nodehwtest/nodehwtest_output_parse.pl --ident $identbase --noerrors | /bin/grep streams","streams_data","overwrite");

    # Try to 'require' the numa_bandwidth.pm module and see if
    # we can use it properly.
    my $modname = "hw_test::numa_bandwidth";
    eval "require($modname)";
    if ($@ =~ /Can't locate hw_test/) {
        print "numa_bandwidth test module not supported.  ($modname not found)\n";
    } elsif ($@) {
        print "Error loading '$modname'.\n\n$@\n";
    }
    else {
        my $tobj = "$modname"->new($ofh);
        if ($tobj) {
            # success! save the module name and object ref
            $$href{$modname} = $tobj;
            defined $DEBUG and print
            "DEBUG: loaded $modname module, test_class=" .
            $tobj->test_class . "\n";
        }
        else {
            print "Error initializing $modname object!\n";
        }

        my $infile = "$destdir/$ident/$hn.snb.numa-bandwidth.out";
        open (IN,"<$infile") or do {
            print "Could not open $infile ($!)\n";
        };
        my @buffer = <IN>;
        close(IN);
        # use the numa_bandwidth.pm-parse() to create a hash full of our data
        my $data = $tobj->parse(\@buffer);


        # digest the raw data
        # the data hash built in numa_bandwidth.pm looks like this:
        # $data{$testname}{$cpu_location}{$mem_location}{$bin_name}->(Statistics::Descriptive object)
        #
        # let's do two things. First, create a row in the full-results table. Second, see if this result qualifies
        # as the best we've seen so we can plot it later
        my $text = "";
        my %best_results = ();
        my %br_range = ();  # parallel hash for %best_results to keep track of range of values
        my %br_bin = (); # parallel hash for %best_results to keep track of which binary did it
        my %full_results = (); # results for every test and every cpu/memory location
        my $num_cpu_locs = 0;
        my $num_mem_locs = 0;
        my $first = 1;
        my $column_headers = "";
        for my $testname (keys %{$data}) {
            ($testname !~ /triad/) and next; # for now we only care about Triad

            $num_cpu_locs = keys(%{$data->{$testname}});

            for my $cpu_loc (sort {$a cmp $b} keys %{$data->{$testname}}) {

                $num_mem_locs = keys(%{$data->{$testname}{$cpu_loc}});

                for my $mem_loc (sort {$a cmp $b} keys %{$data->{$testname}{$cpu_loc}}) {
                    
                    $column_headers .= ",$cpu_loc $mem_loc";

                    for my $bin_name (sort {$a cmp $b} keys %{$data->{$testname}{$cpu_loc}{$mem_loc}}) {

                        my $statref = $data->{$testname}{$cpu_loc}{$mem_loc}{$bin_name};

                        #print "Test Name: $testname, CPU_Loc: $cpu_loc, MEM_Loc: $mem_loc, BIN_Name: $bin_name, mean value: " . $statref->mean() . "\n";
                        ($cpu_loc =~ /\w+=(\d+)/) and my $cpu_num = $1;
                        ($mem_loc =~ /\w+=(\d+)/) and my $mem_num = $1;

                        # save the result as part of the appropriate row
                        $full_results{$testname}{$bin_name} .= "," . $statref->mean();

                        # save this result if it's the best we've seen so far
                        if ((!defined $best_results{$cpu_loc}{$mem_loc}) or ($best_results{$cpu_loc}{$mem_loc} < $statref->mean())) {
                            $best_results{$cpu_loc}{$mem_loc} = $statref->mean();
                            $br_range{$cpu_loc}{$mem_loc} = $statref->sample_range();
                            $br_bin{$cpu_loc}{$mem_loc} = $bin_name;
                            #print "found " . $statref->mean() . " as the new best value for $cpu_loc-$mem_loc\n";
                        }
                            
                    }
                }
            }
        }
        #print "num_cpu_locs: $num_cpu_locs, num_mem_locs: $num_mem_locs\n";
        my $num_cols = $num_cpu_locs*$num_mem_locs;
        #print Dumper (%data);
        #print "Dump of %full_results: " . Dumper(\%full_results) . "\n";
        my %column_values = ();

        # TODO: RKB: come up with a better way to find the best value for each column
        #            I've had the idea to create a 2-D matrix of arrays to keep track of each column,
        #            then find the best value in each column and mark it somehow. This would
        #            obviously require a re-work of the table generation code below.
        #
        #            Having a way to mark best or worst values in a large table seems pretty useful to me.
        #
        # take the %full_results hash and find the best value for each column
#        for my $test (keys %full_results) {
#            #print "test: $test\n";
#            for my $label (keys %{$full_results{$test}}) {
#                my @res_vals = split(',',$full_results{$test}{$label});
#                my $columns = @res_vals;
#                for my $idx (1..($columns-1)) {
#                    $column_values{$idx} = Statistics::Descriptive::Full->new() unless (defined $column_values{$idx});
#                    $column_values{$idx}->add_data($res_vals[$idx]);
#                }
#
#            }
#        }

        #print "Dump of %column_values " . Dumper(\%column_values) . "\n";

        add_section("NUMA Bandwidth Results");
        add_subsection("STREAM Memory Bandwidth Results");

        # build the full results table
        add_table_init("Full NUMA Memory Bandwidth Test Results", $num_cols+1, "3em");

        $column_headers =~ s/physcpubind=/Core~/g;
        $column_headers =~ s/cpunodebind=/Node~/g;
        $column_headers =~ s/membind=/to Node~/g;

        for my $testname (sort {$a cmp $b} keys %full_results) {

            ($testname =~ /triad/i) and add_table_spanning_row("\\textbf{STREAM Triad (MB/s)}");
            add_table_row($column_headers);

            for my $binname (sort {$a cmp $b} keys %{$full_results{$testname}}) {

                add_table_row("$binname" . "$full_results{$testname}{$binname}");
            }
        }
        add_table_conclusion();
        add_text("\\clearpage\n");
    }
}
#
# do the final report generation and such
if ($report) {
	add_section("Cbench Single Node Benchmark Run Details");
	add_text_raw("Hostname of Benchmarked Node: $hn\n".
		"Cbench Test Identifier for Benchmark Run: $ident\n");

	my $tex = "$bench_test/tools/snb_report.tex.in";
	open (IN,"<$tex") or die
		"Could not open $tex ($!)";
	# set slurp mode
	undef $/;
	my $template = <IN>;
	# unset slurp
	$/ = "\n";
	close(IN);

	# title the doc
	my $tmp = "\\title{Cbench Single Node Benchmark Report for Hostname \\texttt{$hn}}";
	$template =~ s/TITLE_HERE/$tmp/gs;

	# write out the core body of the doc
	$tmp = "";
	foreach (@report_core_buf) {
		$tmp .= $_;
	}
	$tmp .= "\n";
	$template =~ s/CORE_BODY_HERE/$tmp/gs;

	# write out the LaTeX source file
	system("/bin/rm -f $destdir/snb_report_$hn.{tex,pdf,log,aux}");
	my $basename = "snb_report_$hn";
	(defined $pdftag) and $basename = "snb_report_$hn\_$pdftag";

	my $tex = "$destdir/$basename.tex";
	open (OUT,">$tex") or die
		"Could not open $tex ($!)";
	print OUT $template;
	close(OUT);

	# try to generate a pdf from the latex input
	my $cmd_output = `pdflatex -interaction=nonstopmode $destdir/$basename.tex 2>&1`;
	if ($cmd_output =~ /Output written on (.*)\.pdf/) {
            print "Created $1.pdf successfully.\n".
            "Check $1.log for details from pdflatex.\n";
        }
        elsif ($cmd_output =~ /LaTeX Error/) {
            print "$cmd_output\n";
        }
	
	# if specified save the raw pieces used to generate the pdf
	if (defined $savepdfsrc) {
		# make a temp directory to hold everything
		my $tmpdir = "/tmp\/$ident";
		mkdir "$tmpdir";
		my @list = (
			"\*.ps",
			"\*.dat",
			"\*.cmd",
			"$basename.tex",
			"snb.$hn.$ident.log",
			"$ident\/$hn\.snb\.\*\.out",
		);
		foreach $f (@list) {
			#print("rsync $f $tmpdir/.\n");
			system("rsync $f $tmpdir/.");
		}
		system("cd /tmp;tar cfz $basename.tgz $ident;/bin/mv -f $basename.tgz $destdir/$basename.tgz");
		system("/bin/rm -rf $tmpdir");
		print "Saved pdf source files in $destdir/$basename.tgz .\n";
	}
}
else {
	logmsg("Finished running the Single Node Benchmarks");
	my $msg = "To generate a report: ".
		"$ENV{'CBENCHOME'}/tools/single_node_benchmark.pl --report ".
		"--node $hn --ident $ident --destdir $destdir\n";
	logmsg($msg);
}
close(LOG);


################################################################
################################################################
########## subroutines

sub addchange_envvar {
	my $buf = shift;
	my $varbase = shift;
	my $value = shift;

	#print ("$$buf\n");
	if ($$buf =~ /[\n]*$varbase\=/) {
		#print "DEBUG:addchange_envvar() varbase found...editing\n";
		$$buf =~ s/([\n]*)$varbase\=.*\n/$1$varbase\=$value\n/g;
	}
	else {
		#rint "DEBUG:addchange_envvar() varbase not found...adding\n";
		$$buf .= "$varbase"."=$value\n";
	}
	#print ("$$buf\n");
}


sub add_list {
	my $name = shift;
	my $list = shift;

	my $text = "$name\n\\begin{itemize}\n";
	foreach (@$list) {
		$text .= "  \\item $_\n";
	}
	$text .= "\\end{itemize}\n";
	push @report_core_buf, $text;
}

sub add_section {
	my $sect = shift;

	push @report_core_buf, "\\section{$sect}\n";
}

sub add_subsection {
	my $subsect = shift;

	push @report_core_buf, "\\subsection{$subsect}\n";
}

sub add_text {
	my $text = shift;

	push @report_core_buf, "$text";
}

sub add_text_raw {
	my $text = shift;

	push @report_core_buf, "\\begin{verbatim}\n$text\\end{verbatim}\n";
}

sub add_figure {
	my $file = shift;
	my $name = shift;
	my $label = shift;

	my ($base,$suf) = $file =~ /(\S+)\.(\S+)$/;
	if (! -f "$destdir/$base.pdf") {
		print "WARNING:add_figure() $base.pdf doesn't exist...not adding figure\n";
		push @report_core_buf, "\nFigure \'$name\' not available.\n\n";
		return;
	}

	my $text = "\n".
"\\begin{figure}[h]\n".
"  \\begin{center}\n".
"  \\includegraphics[scale=0.6]{$file}\n".
"  \\caption{$name}\n".
"  \\label{fig:$label}\n".
"  \\end{center}\n".
"\\end{figure}\n".
"\\clearpage\n";

	push @report_core_buf, "$text";
}

sub add_table_init {

    my $caption = shift;
    $table_num_columns = shift; # declare as global so that other table subroutines can use it
    my $col_width = shift; #optional, if used the data cells will be set to use this width
    my $init_text = "";

    $init_text .= "\\begin{table}[hptb!]\n";
    $init_text .= "\\begin{center}\n";
    $init_text .= "\\caption{$caption}\n";
    $init_text .= "\\begin{tabular}{|r|";
    for my $col (1..($table_num_columns-1)) {
        if (defined $col_width) {
            $init_text .= ">{\\centering}m{$col_width}|";
        }
        else {
            $init_text .= "c|";
        }
    }
    $init_text .= "}\n";

    #print "init_text: $init_text\n";
    push @report_core_buf, $init_text;
}

sub add_table_row {
    # expects a CSV for a row
    my $row = shift;
    $row =~ s/,/ \& /g;

    push @report_core_buf, "$row\\tabularnewline \\hline\n";
}

sub add_table_spanning_row {
    my $cell_text = shift;

    my $spanning_text .= "\\cline{1-" . ($table_num_columns) . "}\n";
    $spanning_text .= "\\multicolumn{" . ($table_num_columns) . "}{|l|}{$cell_text}\\\\ \\hline \\hline\n";

    push @report_core_buf, $spanning_text;
}

sub add_table_conclusion {
    my $concl_text .= "\\end{tabular}\n";
    $concl_text .= "\\end{center}\n";
    $concl_text .= "\\end{table}\n";

    push @report_core_buf, $concl_text;
}

sub build_gnuplot_graph {
	my $fileid = shift;
	my $title = shift;
	my $xlabel = shift;
	my $ylabel = shift;
	my $href = shift;
	my $options = shift;

	# remove old plot files
	system("/bin/rm -f $fileid.{ps,jpg,pdf}");

	open (PLOT, ">$fileid.dat");
	open (CMD, ">$fileid.cmd");

	# build the gnuplot command file we'll run
	print CMD
		"set ylabel \"$ylabel\"\n" .
		"set xlabel \"$xlabel\"\n" .
		"set grid\n" .
		"set title \"$title\"\n" .
		"set term postscript color\n" .
		"set output \"$fileid.ps\"\n";
	
	if (defined $options) {
		foreach (@$options) {
			print CMD "$_\n";
		}
	}

	print CMD "plot ";
	for my $x (sort {$a <=> $b} (keys %$href) ) {
		print PLOT "$xlabel ";
		$keyindex = 2;
		my $numseries = scalar keys(%{$href->{$x}});
		for my $series (sort {$a cmp $b} (keys %{$href->{$x}}) ) {
			print CMD "\"$fileid.dat\" using 1:$keyindex title \"$series\" with lp";
			print PLOT "$series ";
			print CMD ", " unless ($keyindex-1 == $numseries);
			$keyindex++;
		}
		last;
	}
	print CMD "\n";
	print PLOT "\n";

	print CMD
		"set term png\n" .
		"set output \"$fileid.png\"\n" .
		"replot\n" ;

	my $xrm = 
		"-xrm \'gnuplot*line1Color: red\' ".
		"-xrm \'gnuplot*line2Color: green\' ".
		"-xrm \'gnuplot*line3Color: DarkSlateGrey\' ".
		"-xrm \'gnuplot*line4Color: magenta\' ".
		"-xrm \'gnuplot*line5Color: cyan\' ".
		"-xrm \'gnuplot*line6Color: blue\' ".
		"-xrm \'gnuplot*line7Color: orange\' ".
		"-xrm \'gnuplot*line8Color: coral\'";

	my $cmd = "gnuplot $xrm $fileid.cmd";

	# record the command we executed in the gnuplot command file for posterity
	print CMD "\n\# gnuplot command line\n\# $cmd\n";

	# we are done building the command file
	close(CMD);

	# build the data file
	for my $x (sort {$a <=> $b} (keys %$href) ) {
		print PLOT "$x ";
		for my $series (sort {$a cmp $b} (keys %{$href->{$x}}) ) {
			print PLOT $href->{$x}{$series} . " ";
		}
		print PLOT "\n";
	}
	close(PLOT);

	# now execute gnuplot
	(defined $DEBUG) and print "DEBUG:build_gnuplot_graph(): $cmd\n";
	system "$cmd 1>/dev/null 2>&1";

	# generate a pdf from the ps file (hopefully) built by gnuplot
	my $fstats = stat("$fileid.ps");
	if (defined $fstats and $fstats->size > 50) {
		system("ps2pdf $fileid.ps");
	}
	else {
		print "WARNING:build_gnuplot_graph() The generated $fileid.ps seems to be bogus...\n";
	}
}


sub build_gnuplot_histogram {
	my $fileid = shift;
	my $title = shift;
	my $xlabel = shift;
	my $ylabel = shift;
	my $href = shift;
	my $options = shift;

	# we have to have Gnuplot 4.1 or greater for histograms... check for it
	my $tmpbuf = `gnuplot -V`;
	my ($gnuplot_ver) = $tmpbuf =~ /gnuplot\s+(\S+)\s+/;
	if ($gnuplot_ver < 4.1) {
		print "WARNING:build_gnuplot_histogram() requires Gnuplot version 4.1 or greater!\n";
		return;
	}

	# remove old plot files
	system("/bin/rm -f $fileid.{ps,jpg,pdf}");

	open (PLOT, ">$fileid.dat");
	open (CMD, ">$fileid.cmd");

	# build the gnuplot command file we'll run
	print CMD
		"set ylabel \"$ylabel\"\n" .
		"set xlabel \"$xlabel\"\n" .
		"set grid\n" .
		"set title \"$title\"\n" .
		"set term postscript color\n" .
		"set output \"$fileid.ps\"\n".
		"set key outside below right\n".
		"set yrange [0:*]\n".
	    "# histogram specific stuff\n".
		"set auto x\n".
		"set style histogram clustered gap 1 title  offset character 0, 0, 0\n".
		"set style fill   solid 1.00 border -1\n".
		"set datafile missing \'\-\'\n".
		"set style data histograms\n";
		#"set xtics border in scale 1,0.5 nomirror rotate by 0  offset character 0, 0, 0\n".
		#set boxwidth 0.9 absolute

	if (defined $options) {
		foreach (@$options) {
			print CMD "$_\n";
		}
	}

	print CMD "plot ";
	for my $x (sort {$a <=> $b} (keys %$href) ) {
		print PLOT "\"$xlabel\" ";
		$keyindex = 2;
		my $numseries = scalar keys(%{$href->{$x}});
		#plot 'linpack.dat-custom' using 2:xtic(1) ti col, '' u 3 ti col, '' u 4 ti col
		for my $series (sort {$a <=> $b} (keys %{$href->{$x}}) ) {
			print CMD "\"$fileid.dat\" using $keyindex:xtic(1) ti col";
			print PLOT "\"$series threads\" ";
			print CMD ", " unless ($keyindex-1 == $numseries);
			$keyindex++;
		}
		last;
	}
	print CMD "\n";
	print PLOT "\n";

#	print CMD
#		"set term png\n" .
#		"set output \"$fileid.png\"\n" .
#		"replot\n" ;

	my $xrm = 
		"-xrm \'gnuplot*line1Color: red\' ".
		"-xrm \'gnuplot*line2Color: green\' ".
		"-xrm \'gnuplot*line3Color: DarkSlateGrey\' ".
		"-xrm \'gnuplot*line4Color: magenta\' ".
		"-xrm \'gnuplot*line5Color: cyan\' ".
		"-xrm \'gnuplot*line6Color: blue\' ".
		"-xrm \'gnuplot*line7Color: orange\' ".
		"-xrm \'gnuplot*line8Color: coral\'";

	my $cmd = "gnuplot $xrm $fileid.cmd";

	# record the command we executed in the gnuplot command file for posterity
	print CMD "\n\# gnuplot command line\n\# $cmd\n";

	# we are done building the command file
	close(CMD);

	# build the data file
	for my $x (sort {$a <=> $b} (keys %$href) ) {
		print PLOT "$x ";
		for my $series (sort {$a <=> $b} (keys %{$href->{$x}}) ) {
			print PLOT $href->{$x}{$series} . " ";
		}
		print PLOT "\n";
	}
	close(PLOT);

	# now execute gnuplot
	(defined $DEBUG) and print "DEBUG:build_gnuplot_histogram(): $cmd\n";
	system "$cmd 1>/dev/null 2>&1";

	# generate a pdf from the ps file (hopefully) built by gnuplot
	my $fstats = stat("$fileid.ps");
	if (defined $fstats and $fstats->size > 50) {
		system("ps2pdf $fileid.ps");
	}
	else {
		print "WARNING:build_gnuplot_histogram() The generated $fileid.ps seems to be bogus...\n";
	}
}


sub ps2pdf {
	my $file = shift;

	# generate a pdf from the ps file (hopefully) built by gnuplot
	my $fstats = stat("$file");
	if (defined $fstats and $fstats->size > 50) {
		system("ps2pdf $file");
	}
	else {
		print "WARNING:ps2pdf() The generated $file seems to be bogus...\n";
	}
}


sub runcmd {
	my $cmd = shift;
	my $cmdtag = shift;
	my $options = shift;

	my $echo = 0;
	my $finalcmd = $cmd;
	(defined $DEBUG) and $echo = 1;

        # some commands generate files instead of text output
        if (defined $options and $options =~ /file/) {
            if ($cmd =~ /lstopo/) {
                runhwloc($cmd);
            }
        }
        else {
            # need to save the output intelligently
            if (defined $options and $options =~ /overwrite/) {
                    $finalcmd .= " > $destdir/$ident/$hn.snb.$cmdtag.out 2>&1";
            }
            else {
                    $finalcmd .= " >> $destdir/$ident/$hn.snb.$cmdtag.out 2>&1";
            }

            if (defined $options and $options =~ /nosave/) {
                    logmsg("RUNCMD: $cmd",$echo);
                    #system("$cmd") unless defined $dryrun;
                    snb_fork("$cmd") unless defined $dryrun;
            }
            else {
                    logmsg("RUNCMD: $finalcmd",$echo);
                    #system("$finalcmd") unless defined $dryrun;
                    snb_fork("$finalcmd") unless defined $dryrun;
            }
        }
        ($main::INTsignalled) and exit(1);
}

sub snb_fork {

    my $cmd = shift;

    # use fork so that we can kill the child command if necessary
    my $pid = fork();
    if (not defined $pid) {
        print "resources not avilable.\n";
    } elsif ($pid == 0) {
        exec("exec $cmd"); # the IO redirection in $cmd will cause Perl to create a 
                           # sub-process unless 'exec' is prepended to the command
                           # http://stackoverflow.com/questions/2965067/problem-with-fork-exec-kill-when-redirecting-output-in-perl
        exit(0);
    } else {
        $main::childpids{$pid} = 1;
        waitpid($pid,0);
    }
    delete($main::childpids{$pid}); # don't need to try to kill children that have finished
}

sub logmsg {
	my $msg = shift;
	my $echo = shift;

	my $txt = get_log_timestamp()." $msg\n";
	print LOG $txt;
	print $txt unless (defined $echo and $echo == 0);
}

sub get_file_timestamp {
	my ($sec, $min, $hour, $day, $mon, $year, $wday, $yday, $isdst) = localtime;
    
    $year = $year + 1900;
    $stamp = sprintf "%04d-%02d-%02d_%02d:%02d:%02d",$year,$mon+1,$day,$hour,$min,$sec;
    return $stamp;
}

sub get_log_timestamp {
	my ($sec, $min, $hour, $day, $mon, $year, $wday, $yday, $isdst) = localtime;
    
    $year = $year + 1900;
    $stamp = sprintf "%04d/%02d/%02d %02d:%02d:%02d",$year,$mon+1,$day,$hour,$min,$sec;
    return $stamp;
}

sub runhwloc {
    my $cmd = shift;

    # Generate either a PDF or text file containing the memory hierarchy for a node
    # default to txt
    my $suffix = "txt";

    # the lstopo help file shows avaliable file formats based on how it was configured and built
    my $lstopo_help = `$cmd --help 2>&1`;
    
    # if it says it can make an xml file, do it (that way we can generate other output file types later)
    if ($lstopo_help =~ /Supported output file formats:.*xml/) {
        $suffix = "xml";
    }
    my $finalcmd = "$cmd $destdir/$ident/$hn.snb.lstopo.$suffix";

    logmsg("RUNCMD: $finalcmd",$echo);
    system("$finalcmd") unless defined $dryrun;

    # if it says it can make a pdf, make a pdf
    if ($lstopo_help =~ /Supported output file formats:.*pdf.*/) {
        $suffix = "pdf";
    }

    $finalcmd = "$cmd $destdir/$ident/$hn.snb.lstopo.$suffix";

    logmsg("RUNCMD: $finalcmd",$echo);
    system("$finalcmd") unless defined $dryrun;
}

sub usage {
    print "USAGE: $0 \n" .
    "Cbench utility to benchmark a single node and generate a LaTeX report of the results\n" .
    "\n" .
    " General Cbench Options:\n" .
    "   --tests <regex>     Run only tests whose test name is included in the\n".
    "                       provided regex string. For example:\n".
    "                         --tests 'stream|cachebench|linpack'\n".
    "   --ident <string>    Identifier for the test group\n" .
    "   --binident <string> Identifier for the set of binaries to use.\n" .
    "                       Maps to $CBENCHTEST/<binident> .\n".
    "   --numcpus <num>     Override the auto-detection of the number of CPU\n".
    "                       cores\n".
    "   --dryrun            Do everything but actually run the tests\n" .
    "   --quiet             Output as little as possible during run\n".
    "   --debug <level>     Turn on debugging at the specified level\n" .
    "\n" .
    " Single Node Benchmark Options:\n" .
    "   --destdir           Specify the desitnation directory for the *.snb*.out files\n" .
    "   --run               Actually run the tests, if defined (default=defined)\n" .
    "   --report            Generate a report, if defined (default=undef)\n" .
    "   --savepdfsrc        Save intermediate files when generating report, if defined (default=undef)\n" .
    "   --pdftag            An arbitrary tag appended to the report pdf filename\n" .
    "   --node              The hostname of a specific node that is to be tested\n" .
    "   --help              Print this help information\n" .
    "   --nodetonode        Run NUMA tests from each memory node to each memory nodes (default)\n" .
    "   --coretonode        Run NUMA tests from each core to each memory node\n" .
    "";

# RKB: I think these would be useful features to have in the SNB script (particularly since
# they're supported by node_hw_test)
#    "   --iterations <num> Number of test iterations\n" .
#    "   --exclude <regex>  Do NOT run hw_test modules that match the\n" .
#    "                      the specified regex string. For example,\n" .
#    "                         --exclude streams\n" .
#    "                      would only run the streams hw_test module\n" .
#    "   --maxmem <num>     Override the auto-detection of free memory to be\n".
#    "                      used in testing. The value is the number of megabytes.\n".
#    "                      For example,\n".
#    "                         --maxmem 1024\n".
#    "                      would only use 1024MB or 1GB of memory for any tests\n".
#    "                      that utilize free memory detection.\n".
#    "   --memory_util_factors  Override the cluster.def \@memory_util_factors array.\n".
#    "                          For example:\n".
#    "                            --memory_util_factors 0.10,0.77,0.85\n".
}

sub run_numa_tests {
    my $prefix = shift;
    my $test = shift;
    my $mode = shift;
    my $filename = shift;
    my $date = `date +%d%b%Y_%H%M`;
    chomp($date);

    debug_print(3,"DEBUG:In run_numa_tests()\n");
    debug_print(3,"DEBUG:  prefix: $prefix\n");
    debug_print(3,"DEBUG:  test: $test\n");
    debug_print(3,"DEBUG:  mode: $mode\n");
    debug_print(3,"DEBUG:  filename: $filename\n");

    # find numactl
    my $numactl = "numactl";

    #   look for numactl in our current path
    my $ret = `which $numactl`;
    if ($ret =~ /^which: no/ or $ret =~ /^$/) {
        # maybe it's in /usr/bin
        if (-e "/usr/bin/numactl") {
            $numactl = "/usr/bin/numactl";
        }
        # can't find it - abort this subroutine
        else {
            print STDERR "run_numa_tests(): can't find $numactl\n";
            return;
        }
    }
    debug_print(3, "DEBUG:  using $numactl\n");

    # check numactl's capabilities
    $ret = `$numactl 2>&1`;
    my $bind_process_to_node = "cpunodebind";
    ($ret =~ /--cpubind=/) and $bind_process_to_node = "cpubind";
    if (($ret !~ /membind/) or ($ret !~ /preferred/) or ($ret !~ /physcpubind=/)) {
        print STDERR "run_numa_tests(): some functionality of $numactl is missing\n";
        return;
    }

    # since we're using numactl, gather the number of nodes and cores that numactl sees
    my $numa_show = `$numactl --show 2>&1`;
    my $numa_hardware = `$numactl --hardware 2>&1`;
    my $numa_max_node = 1;
    my $numa_max_core = 1;
    my @cores = ();
    my @nodes = ();
    ($numa_show =~ /physcpubind: (.*)\n/) and @cores = split(' ',$1);
    ($numa_show =~ /nodebind: (.*)\n/) and @nodes = split(' ',$1);
    $numa_max_node = pop(@nodes);
    $numa_max_core = pop(@cores);
    debug_print(3, "DEBUG:  numa_max_core: $numa_max_core\n");
    debug_print(3, "DEBUG:  numa_max_node: $numa_max_node\n");

    system("echo `date` >> $filename");
    # run the command according to the requested mode
    if ($mode eq "node-to-node") {
        debug_print(3, "DEBUG:  Running node-to-node tests\n");
        # run the test command running on each memory node with its memory on each memory node
        for my $cpu_node (0..$numa_max_node) {
            for my $mem_node (0..$numa_max_node) {
                my $command = "$prefix $numactl --$bind_process_to_node=$cpu_node --membind=$mem_node $test";
                #print "run_numa_test() command: $command\n";
                system("echo \"CBENCH RUN_NUMA_TEST COMMAND: $command\" >> $filename");
                snb_fork("$command >> $filename 2>&1") unless defined $dryrun;


#                system("echo \"CBENCH RUN_NUMA_TEST COMMAND: $command\" >> $filename; $command >> $filename 2>&1") unless $dryrun;
            }
        }

    }
    elsif ($mode eq "core-to-node") {
        debug_print(3, "DEBUG:  Running core-to-node tests\n");

        # run the test command running on each core with its memory on each memory node
        for my $core (0..$numa_max_core) {
            for my $mem_node (0..$numa_max_node) {
                my $command = "$prefix $numactl --physcpubind=$core --membind=$mem_node $test";
                system("echo \"CBENCH RUN_NUMA_TEST COMMAND: $command\" >> $filename");
                snb_fork("$command >> $filename 2>&1") unless defined $dryrun;
            }
        }
    }
    else {
        print STDERR "run_numa_tests(): Argument 3 must be either 'node-to-node' or 'core-to-node'\n";
        return;
    }

}

