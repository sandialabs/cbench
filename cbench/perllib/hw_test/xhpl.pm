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


package hw_test::xhpl;

use strict;

my $package = __PACKAGE__;
my ($shortpackage) = $package =~ /hw_test::(\S+)/;

=head1 NAME

hpcc

Cbench hw_test module that uses the HP Linpack benchmark on a single
node to test out cpu and memory performance.
This module will only work if the Cbench local mpich version of
Linpack (xhpl) has been compiled and installed into the nodehwtest test
set.

=head1 Cbench hw_test SYNOPSIS

This is a module for the Cbench hw_test framework. The Cbench hw_test
framework uses simple Perl object modules like this to provide "plug-in"
hardware testing functionality.  Each module encapsulates the intelligence
to run a certain hardrware test or closely related tests and also 
analyze the output from the tests and summarize the data. Cbench utilities
like node_hw_test use these modules.

These hw_test modules are written as Perl objects simply because it was
a good clean way to deal with the dynamic "plug-in"  type model the Cbench
hw_test framework is using.  One shouldn't be scared by the fact that they
are objects.  The object oriented usage is very basic.  Most of the methods
are generic and won't need to be changed for any new hw_test modules that
are written.

See doc/README.hw_test for more information on the Cbench hardware
testing framework.

=cut

###############################################################
#
# Public methods
#


=head1 PUBLIC METHODS

=over 4

=item B<new()> - Create a new hw_test object

Should be called with a parameter that is an open filehandle.

  $obj = hw_test::cpuinfo->new(*FH);

The filehandle parameter is where output from the testing encapsulated
in this hw_test object is printed to. If no parameter is given,
STDOUT is assumed.

=cut

sub new {
	my $type = shift;
	my $self = {};
	bless $self, $type;
	$self->_init(@_) or return;

	return $self;
}


=item B<run()> - Responsible for actually running the test(s)
that this hw_test module encapsulates.

  $obj = hw_test::cpuinfo->new(*FH);
  $obj->run();

The output from tesstreamsting should be sent to the output file
handle for the object, $self->{outhandle}.

The implementation of the run() routine/method for a hw_test
module encompasses the first 50% of the work in creating a new
hw_test module.

=cut

# private variable, how many iterations do we run each time
# run() is called
my $iterations = 1;

sub run {
	my $self = shift;
	
	# grab the output file handle for the object
	my $ofh = $self->{outhandle};

	# path to the binaries
	my $path = "$main::bench_test/$main::TESTBINPATH";
		
	my @buf = ();
	
	# find the appropriate mpirun to use
	my $mpirun = main::find_bin('mpirun',"$main::bench_test/mpich/bin","$main::BENCH_HOME/opensource/mpich/bin");
	(!defined $mpirun) and do {
		print "$shortpackage\.run() Couldn't find mpirun...exiting\n";
		return 1;
	};

	# We have to create a little sandbox for linpack to run in.
	use File::Temp;
	
	# get a temporary directory, automatically clean it up when done
	my $rundir = File::Temp::tempdir(CLEANUP => 1);
	my $pwd = `/bin/pwd`;

	(defined $main::DEBUG) and print
		"DEBUG:$shortpackage\.run() temp directory = $rundir\n";
	
	# build the requisite content in the directory for HPCC to run
	#
	chdir $rundir;
	
	# build the HPL.dat input file for linpack
	# read in config file generation template
	my $file = "$main::testset_path\/xhpl_dat.in";
	open (IN,"<$file") or die
		"Could not open $file ($!)";
	undef $/;
	my $xhpldat = <IN>;
	close(IN);
	$/ = "\n";

	# search and replace 
	my @Nvals = main::compute_N(1,1);
	my $num_Nvals = @Nvals;
	my $outbuf = $xhpldat;	
	$outbuf =~ s/XHPL_NUM_N_HERE/$num_Nvals/gs;
	$outbuf =~ s/XHPL_N_HERE/@Nvals/gs;
    $outbuf =~ s/XHPL_P_HERE/1/gs;
    $outbuf =~ s/XHPL_Q_HERE/1/gs;

	# write out the generated hpccinf.txt file
	$file = "$rundir\/HPL.dat";
	open (OUT,">$file") or do {
		print "ERROR:$shortpackage.run() Could not write $file ($!)\n";
		return;
	};
	print OUT $outbuf;
	close(OUT);

	# need a simple hosts file for mpirun
	$file = "$rundir\/hostlist";
	open (OUT,">$file") or do {
		print "ERROR:$shortpackage.run() Could not write $file ($!)\n";
		return;
	};
	print OUT "$main::hn\n";
	print OUT "$main::hn\n";
	close(OUT);

	#
	# Ok, should be ready to run Linpack
	my $cmd = "$mpirun -machine shmem ".
		"-machinefile $rundir\/hostlist -np 1 $path/xhpl.ch_shmem";
	(defined $main::DEBUG) and print
		"DEBUG:$shortpackage\.run() cmd = $cmd\n";
	main::run_single_process("$cmd",\@buf);

	# save output
	print $ofh @buf;

	chdir $pwd;
}

=item B<parse()> - Responsible for actually parsing the output
from the test(s) that this hw_test module encapsulates and 
returning a hash reference with the extracted data.

parse() takes a reference to a buffer (array) as the single input

  $obj = hw_test::cpuinfo->new(*FH);
  $obj->run();
  .
  read the output from run() in from the output file it
  went to and put it in @buf
  .
  $datahashref = $obj->parse(\@buf);
  for $k (keys %{$datahashref}) {
  print "$k => $datahashref->{$k}\n";
  }

The implementation of the parse() routine/method for a hw_test
module encompasses the second 50% of the work in creating a new
hw_test module.

=cut

sub parse {
	my $self = shift;
	my $bufref = shift;

	my %data;
	my $key;
	my $fail = 0;

	# prepare to parse the buffer
	my $txtbuf = \@$bufref;
	my $numlines = scalar @{$txtbuf}; 

	# NOTE: The linpack output file can have multiple results within it.
	#       We want to grab the maximum PASSED result from the file. We
	#       will only record the linpack data into the final data hash
	#       if all results within the file PASSED residual checks.
    my $status = 'NOTSTARTED';
    my $found_result = 0;
	my $found_endrecord = 0;
    my $i = 0;
	my $gflops = 'NODATA';
	my $local_max_gflops = 0.0;
	my $local_total_tests = 'nodata1';
	my $local_passed_tests = 'nodata2';
	my $local_failed_tests = 'nodata3';
	my $total_time = 0;
    while ($i < $numlines) {
        ($txtbuf->[$i] =~ /matrix A is randomly generated/) and $status = 'STARTED';

		#HPL ERROR from process # 0, on line 170 of function HPL_pdtest:
		#>>> [39,47] Memory allocation failed for A, x and b. Skip. <<<
        ($txtbuf->[$i] =~ /Memory allocation failed/) and $status = 'ALLOCFAILURE';

		($txtbuf->[$i] =~ /T\/V\s+N\s+NB.*$/) and $found_result = 1 and
			$status = 'FOUND A RESULT';

		($txtbuf->[$i] =~ /Finished.*$/) and $found_endrecord = 1 and do {
			$status = 'FINISHED' unless $status =~ /ALLOC/;
		};

		$found_endrecord and goto xhplendrecord;

		if (!$found_result) {
			$i++;
			next;
		}

        # we need at least 6 more lines in the buffer to make
        # a determination for this chunk of results. we do
        # this check to keep from overflowing our buffer as
        # we parse....
		if ($i + 6 > $numlines) {
			(defined $main::DEBUG and $main::DEBUG > 2) and print
				"parsing ended, buffer would overflow\n";
			last;
		}

		# the beginning of a result record was found, the actual
		# result gigaflops number is 2 lines down in the buffer
		$i += 2;
        my $l =  $txtbuf->[$i];
        chomp $l;
		my ($exp, $mantissa, $exp_str);
		# matches a result line with Gflops in exponent notation
        if ($l =~ /(\S+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+\.\d+)\s+(\d+\.\d+)e\+(\d+)/) {
			$total_time += $6;
			$exp = $8;
			$mantissa = $7;
			$exp_str = 'e+';
			$gflops = $mantissa * (10 ** $exp);
        }
		# matches a result line without exponent notation
        elsif ($l =~ /(\S+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+\.\d+)\s+(\d+\.\d+)/) {
			$total_time += $6;
			$mantissa = $7;
			$exp = 'noexp';
			$gflops = $mantissa;
			$exp_str = 'noexp';
        }


		# check for the PASSED or FAILED status of the result
		$i += 2;
		my $pass = 0;
		$pass = ($txtbuf->[$i++] =~ /PASSED/);
		$pass += ($txtbuf->[$i++] =~ /PASSED/);
		$pass += ($txtbuf->[$i] =~ /PASSED/);

		# if the parsed Gigaflops result passed and if the result is the max
		# of any previous results from this file, then record it locally
		# pending the determination whether the overall output file is deemed
		# to pass (this check is done further down)
		if ($pass == 3) {
			if ($gflops > $local_max_gflops) {
				$local_max_gflops = $gflops;
			}
		}

		(defined $main::DEBUG and $main::DEBUG > 2) and print
			"RESULT, $gflops, $mantissa, $exp, $exp_str, $pass, $status\n";

		# we finished parsing a test result, prime the loop for finding the next
		# result in case there are multiple results in the output
		$found_result = 0;
		$i++;
		next;

xhplendrecord:
        $l =  $txtbuf->[$i];
        chomp $l;
        if ($l =~ /Finished\s+(\d+)\s+tests with the following.*$/) {
			$local_total_tests = $1;
		}
		elsif ($l =~ /\s+(\d+)\s+tests completed and passed.*$/) {
			$local_passed_tests = $1;
		}
		elsif ($l =~ /\s+(\d+)\s+tests completed and failed.*$/) {
			$local_failed_tests = $1;
		}			

		$i++;
	}

	(defined $main::DEBUG and $main::DEBUG > 2) and print
		"ENDPARSE, $local_max_gflops, $local_total_tests, $local_passed_tests, ".
		"$local_failed_tests, $status\n";

	# only if all tests in the output file PASSED do we flag this overall
	# linpack benchmark as having completed correctly
	if ($status =~ /FINISHED/ and $local_total_tests == $local_passed_tests) {
		$status = 'COMPLETED';
	}
	elsif ($status =~ /ALLOCFAILURE/) {
		$status = 'ALLOCFAILURE';
	}
	elsif ($status =~ /FINISHED/ and $local_passed_tests < $local_total_tests) {
		$status = 'FAILED RESIDUALS';
	}
	else {
		$status = 'FAILED'
	}

	(defined $main::DEBUG and $main::DEBUG > 2) and print
		"ENDSTATUSCHECK, $status\n";

	if ($status =~ /COMPLETED/) {
		$key = "$self->{SHORTNAME}\_gflops";
		$data{$key} = $local_max_gflops;
		$key = "$self->{SHORTNAME}\_runtime";
		$data{$key} = $total_time / 60;
	}
	else {
		$key = "$self->{SHORTNAME}\_fail";
		$data{$key} = $local_failed_tests;
	}

	return \%data;
}

=item B<name()> - Return the name of the hw_test module

For example: hw_test::cpuinfo

=cut

sub name {
	my $self = shift;
	return $self->{NAME};
}

=item B<test_class()> - Return the test_class of the hw_test module

For example: cpu, memory, disk, ...

=cut

sub test_class {
	my $self = shift;
	return $self->{TEST_CLASS};
}



###############################################################
#
# "Private" methods
#

=head1 PRIVATE METHODS

=over 4

=item B<_init> - parse arguments for B<new()>

Parse the arguments passed to B<new()> and sets the
appropriate object variables and such.

=cut

sub _init {
	my $self = shift;
	if (@_ == 0) {
		$self->{outhandle} = *STDOUT;
	}
	elsif (@_ == 1) {
		# Single parameter, better be a filehandle.
		# save the handle in the object
		$self->{outhandle} = shift;
	} else {
		my %args = @_;
		map { $self->{$_} = $args{$_}; } keys %args;
	}

	# this defines the Cbench hw_test test class (i.e. cpu, memory, etc.)
	# for this dude
	$self->{TEST_CLASS} = 'cpu';
	# save our name so callers can ask us
	$self->{NAME} = $package;
	$self->{SHORTNAME} = $shortpackage;
	
	return 1;
}


1;
