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
use lib ($ENV{CBENCHOME} ? $ENV{CBENCHOME} : "$ENV{HOME}\/cbench");
$BENCH_HOME = $ENV{CBENCHOME} ? $ENV{CBENCHOME} :
	"$ENV{HOME}\/cbench";
require "cbench.pl";

# add Cbench perl library to the Perl search path
use lib ($ENV{CBENCHOME} ? "$ENV{CBENCHOME}\/perllib" :
	"$ENV{HOME}\/cbench\/perllib");

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
);

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
# in each testset for whatever we are doing. This the basename for those
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

(defined $run) and
	logmsg("INITIATING Single Node Benchmarking RUN on node $hn, test identifier is $ident");
(defined $report and !defined $run) and
	logmsg("INITIATING Single Node Benchmarking REPORT for node $hn, test identifier is $ident");

# grab some info on the node
if (defined $run) {
	runcmd("/bin/uname -s -r -m -p -i -o","uname","overwrite");
	runcmd("cat /proc/cpuinfo","cpuinfo","overwrite");
	runcmd("cat /proc/meminfo","meminfo","overwrite");
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
print Dumper(%cpumap);
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
if ($tests =~ /streams/ and $run) {
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
			runcmd("export OMP_NUM_THREADS=$numcores; $binpath/nodeperf2-nompi -i $i -s $n","nodeperf2");
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

	# We tweak the OMP_NUM_THREADS environment var to control the
	# number of threads used. We do this by modifying the common_header.in
	# template in $CBENCHTEST. Before we muck with this file, save the
	# original.
	runcmd("/bin/cp -f $bench_test/common_header.in $bench_test/common_header.in.SNB_SAVED",'','nosave');

	# read in the common_header template
	my $tmp = "$bench_test/common_header.in.SNB_SAVED";
	open (IN,"<$tmp") or die
		"Could not open $tmp ($!)";
	# set slurp mode
	undef $/;
	my $orig = <IN>;
	# unset slurp
	$/ = "\n";
	close(IN);

	# do the actual job script generation
	for my $threads (1..$numcores) {
		# write out a common_header.in template with the right number of threads
		$cmnhdr = $orig;
		addchange_envvar(\$cmnhdr,"export OMP_NUM_THREADS","$threads");
		my $out = "$bench_test/common_header.in";
		open (OUT,">$out") or do {
			print "Could not open $out ($!)\n";
		};
		print OUT "$cmnhdr\n";
		close(OUT);

		my $count = $numcores / $threads;
		for my $processes (1..$count) {
			#(($processes * $threads) > $numcores) and next;
			(!power_of_two($processes) and ($processes > 1)) and next;
			(!power_of_two($threads) and ($threads > 1)) and next;

			my $pq = $threads * $processes;
			my $cmd = "$bench_test/linpack/linpack_gen_jobs.pl --ident ".$identbase."_".
				$threads."threads --runsizes $processes $binidentopt";
			#print "$cmd\n";
			runcmd("$cmd",'linpack');
		}
	}
	# restore the common_header.in template to its original state
	runcmd("/bin/cp -f $bench_test/common_header.in.SNB_SAVED $bench_test/common_header.in",'','nosave');

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
	#print Dumper (%data);

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
	add_text("Memory Utilization Factors Used: $memfactors\n");
	add_figure("linpack.pdf","Linpack Data","linpack");

	# now, normalize the data by number of MPI processes and build another
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
	runcmd("$bench_test/npb/npb_output_parse.pl --match \'$identbase\' --normalize numprocs ".
		"--collapse \'snb\' --xrange 1,$num_cores_counted --gnuplot --yrange 0,\* ".
		"--linewidth 4 </dev/null","npb");
	runcmd("/bin/cp -f $bench_test/npb/npb.ps $destdir/npb-normalized.ps","npb");
	ps2pdf("$destdir/npb-normalized.ps");
	add_text("This graph normalizes the NAS Parallel Benchmark data by the number of ".
		"MPI processes so that scaling efficiency can be seen.\n");
	add_figure("npb-normalized.pdf","Normalized NAS Parallel Benchmark Data","npb-normalized");
}


# hpcc, use testset, need cbench tweaked version with cmdline options

#
# test out how locking down threads to various core combinations
# affects performance. this kind of tests linux scheduler prowess
# as well as other core locality things. the data is quite open for
# interpretation...



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
	system("pdflatex -interaction=nonstopmode $destdir/$basename.tex 1>/dev/null 2>&1");
	(-f "$destdir/$basename.pdf") and
		print "Created $destdir/$basename.pdf successfully.\n".
		"Check $destdir/$basename.log for details from pdflatex.\n";
	
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
		push @report_core_buf, "Figure \'$name\' not available.\n";
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
		for my $series (sort {$a cmp $b} (keys %{$href->{$x}}) ) {
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
		for my $series (sort {$a cmp $b} (keys %{$href->{$x}}) ) {
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

	# need to save the output intelligently
	if (defined $options and $options =~ /overwrite/) {
		$finalcmd .= " > $destdir/$ident/$hn.snb.$cmdtag.out 2>&1";
	}
	else {
		$finalcmd .= " >> $destdir/$ident/$hn.snb.$cmdtag.out 2>&1";
	}

	if (defined $options and $options =~ /nosave/) {
		logmsg("RUNCMD: $cmd",$echo);
		system("$cmd") unless defined $dryrun;
	}
	else {
		logmsg("RUNCMD: $finalcmd",$echo);
		system("$finalcmd") unless defined $dryrun;
	}
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
