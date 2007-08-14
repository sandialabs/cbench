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

# This is the Cbench template script for starting jobs in testsets
# that use job directories as opposed to job files, e.g. linpack
# testset

# need to know where everything cbench lives!
use lib ($ENV{CBENCHOME} ? $ENV{CBENCHOME} : "$ENV{HOME}\/cbench");
$BENCH_HOME = $ENV{CBENCHOME} ? $ENV{CBENCHOME} :
    "$ENV{HOME}\/cbench";
require "cbench.pl";

use Getopt::Long;

GetOptions( 'ident=s' => \$ident,
            'batch' => \$batch,
            'interactive' => \$inter,
            'throttledbatch=i' => \$throttledbatch,
            'serialbatch' => \$serialbatch,
            'waitall' => \$waitall,
            'match=s' => \$match,
            'minprocs=i' => \$minprocs,
            'maxprocs=i' => \$maxprocs,
			'procs=i' => \$procs,
			'batchargs=s' => \$batchargs,
			'repeat=i' => \$repeat,
            'delay=i' => \$delay,
            'polldelay=i' => \$polldelay,
            'dryrun' => \$DRYRUN,
			'debug:i' => \$DEBUG,
            'help' => \$help,
          );

$minprocs=2 unless $minprocs;
if (defined $help) {
    usage();
    exit;
}

(!defined $ident) and $ident = $cluster_name . "1";
(defined $serialbatch) and $throttledbatch = 1;

if (!defined $batch and !defined $inter and !defined $throttledbatch) {
    die "--batch or --interactive or --throttledbatch parameter required\n";
}

if (defined $waitall and !defined $throttledbatch) {
	die "--waitall can only be used with --throttledbatch or --serialbatch";
}

(!defined $match) and $match = ".*";
(!defined $delay) and $delay = "1";
(!defined $polldelay) and $polldelay = "120";
(!defined $repeat) and $repeat = "1";
(!defined $batchargs) and $batchargs = " ";

if (defined $procs) {
	$minprocs = $procs;
	$maxprocs = $procs;
}

(defined $batch) and $start_method  = 'batch';
(defined $inter) and $start_method  = 'interactive';
if (defined $throttledbatch) {
	$start_method  = 'throttledbatch';
	(defined $waitall) and $start_method .= "-waitall";
	$SIG{USR1} = sub { $main::DEBUG++; };
	$SIG{USR2} = sub { $main::DEBUG--; };
}

$pwd = `pwd`;
chomp $pwd;
$bench_test = get_bench_test();
$testset = 'TESTSET_NAME_HERE';

# we may have multiple idents specified..
my @identlist = split(',',$ident);
debug_print(1,"DEBUG: list of idents: ". join(' ',@identlist). " \n");
foreach my $i (@identlist) {
	$testpath = "$bench_test/$testset/$i";
	chdir $testpath;

	print "Starting jobs for test identifier \'$i\':\n";
	start_jobs($start_method,$match,$delay,$polldelay,$maxprocs,$minprocs,$repeat,$batchargs,$throttledbatch);

	chdir $pwd;
}


sub usage {
    print "USAGE: $0\n" .
          "    --batch                Submit batch jobs\n" .
          "    --interactive          Start interactive jobs\n" .
          "    --throttledbatch <num> Start batch jobs in a throttled mode where the\n" .
		  "                           numeric parameter is the total number of queued\n".
		  "                           and running jobs that Cbench will submit to the\n".
		  "                           batch system. For example:\n".
		  "                             --throttledbatch 8\n".
		  "                           would only run a single job at a time via the batch\n".
		  "                           scheduler.\n".
		  "    --serialbatch          Serialized batch submission mode which is equivalent\n".
		  "                           to invoking with\n".
		  "                             --throttledbatch 1\n".
		  "    --waitall              A throttledbatch mode option that will cause start_jobs\n".
		  "                           to not exit until all jobs submitted have left the \n".
		  "                           batch system\n".
          "    --match <regex>        This limits the starting up of jobs to jobs with a\n" .
          "                           jobname that contains the specified string. For example,\n" .
          "                             --match 2ppn\n" .
          "                           would only start 2 ppn tests\n" .
          "    --minprocs <num>       The minimum number of processes to use to run jobs\n" .
          "                           NOTE: to run 1 processor jobs you MUST specify\n".
          "                             --minprocs 1\n".
          "                           By default the minimum job size is 2 processors\n".
          "    --maxprocs <num>       The maximum number of processes to use to run jobs\n" .
		  "    --procs <num>          Shortcut to set --minprocs and --maxprocs to the\n".
		  "                           same value, i.e. to run jobs with only a single\n".
		  "                           number of processors (not a range)\n".
          "    --batchargs 'string'   Pass these arguments on the commandline when\n".
          "                           batch jobs are started\n".
		  "    --repeat <num>         Start the set of jobs specified <num> times\n" .
          "    --delay <num>          The number of seconds to sleep between jobs\n" .
          "    --polldelay <num>      The number of seconds to sleep between polls of\n" .
		  "                           the batch system in throttledbatch mode\n".
		  "    --debug <level>        Debug level\n".
          "    --dryrun               Do everything but start jobs to see what would happen\n";
}
