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


package output_parse::imb;

use strict;

my $package = __PACKAGE__;
my ($shortpackage) = $package =~ /output_parse::(\S+)/;

=head1 NAME

imb

Cbench parse module responsible for parsing output files generated
by the Intel MPI Benchmarks (aka Pallas Benchmarks in an older form)
.

=cut

###############################################################
#
# Public methods
#


=head1 PUBLIC METHODS

=over 4

=item B<new()> - Create a new parse object

  $obj = output_parse::imb->new();

=cut

sub new {
	my $type = shift;
	my $self = {};
	bless $self, $type;
	$self->_init(@_) or return;

	return $self;
}


=item B<parse()> - Responsible for actually parsing the output
contained in an output file and returning a hash reference with
the extracted data.

parse() takes a reference to a buffer (array) as the single input

  $obj = output_parse::imb->new();
  .
  read in an output file into @buf
  .
  $datahashref = $obj->parse(\@buf);
  for $k (keys %{$datahashref}) {
  print "$k => $datahashref->{$k}\n";
  }

The implementation of the parse() routine/method for a parse
module encompasses most of the work in creating a new
parse module.

=cut

sub parse {
	my $self = shift;
	# a string identifying the file(s) we are parsing output from,
	# used mainly for printing out things
	my $fileid = shift;
	# should be one or more buffer references passed in,
	# they are ordered according to our very own FILE_LIST
	# array (see file_list() below)
	my @bufrefs = @_;

	my %data;

	# process all the lines in the STDOUT buffer
	my $txtbuf = $bufrefs[0];
	my $numlines = scalar @$txtbuf; 

	# Parsing IMB output is a bit of a pain. It can have several
	# different tests run that output data in table formats with
	# varying columns. We'll use a simple state machine controlled
	# by $parse_state.
	#
	# State Table:
    # 100 - looking for the benchmarks to run table
    # 101 - parsing the benchmarks to run table
	# 	0 - looking for next benchmark table
	#   1 - looking for process count in benchmark table
	#   2 - reading PingPong or PingPing table
	#   3 - reading SendRecv table
	#   4 - reading collective test tables (not Barrier)
	#   5 - reading Barrier table
    my $status = 'NOTSTARTED';
	my $parse_state = 100;
	my $testname = '';
    my $skipblankline = 0;
    my %tests;
    my $error = 0;
    foreach my $l (@{$txtbuf}) {
        ($l =~ /MPI Benchmark Suite /) and $status = 'STARTED';
		($l =~ /waiting in MPI_Barrier/) and next;
		($l =~ /\#\-\-\-\-\-\-\-\-\-\-\-/) and next;

		if ($parse_state == 100 and $l =~ /List of Benchmarks to run/) {
        	$parse_state = 101;
            $skipblankline = 1;
            
            (defined $main::DEBUG and $main::DEBUG > 2) and print
				"DEBUG:imb: parse_state=$parse_state\n";
        }
        elsif ($parse_state == 101) {
        	# skip the first blank line after the title line
        	if ($l !~ /\#/  and $skipblankline) {
            	$skipblankline = 0;
                next;
            }
            elsif ($l !~ /\#/) {
            	$parse_state = 0;
                
                if (defined $main::DEBUG and $main::DEBUG > 2) {
                	foreach my $k (keys %tests) {
                    	print "DEBUG:imb:testtorun $k\n";
                    }
                }
            }
            elsif ($l =~ /\# (\S+)$/) {
            	$tests{$1} = 0;
            }
        }
		#elsif ($parse_state == 0 and $l =~ /Benchmarking (\S+)\s*/) {
		elsif ($l =~ /Benchmarking (\S+)\s*/) {
			# found the start of a new benchmark table
			$testname = $1;
			$parse_state = 1;
            $tests{$testname} = 1;
		}
		elsif ($parse_state == 1 and $l =~ /processes = (\d+)\s*/) {
			# found the process count for the test, ignore tests
			# where the process count does not equal the number of
			# processors in the Cbench jobname
			if ($1 == $main::np or ($1 == 2 and $testname =~ /Ping/)) {
				($testname =~ /Ping/) and $parse_state = 2;
				($testname =~ /Sendrecv|Exchange/) and $parse_state = 3;
				($testname =~ /Allreduce|Reduce|Reduce_scatter|Allgather|Allgatherv|Alltoall|Bcast/)
					and $parse_state = 4;
				($testname =~ /Barrier/) and $parse_state = 5;
			}
			else {
				$parse_state = 0;
			}

			(defined $main::DEBUG and $main::DEBUG > 2) and print
				"DEBUG:imb: parse_state=$parse_state\n";
		}
		elsif ($parse_state == 2) {
			($l =~ /\#bytes\s+\#repetitions/) and next;
			$l =~ /\s+(\d+)\s+(\d+)\s+(\d+\.\d+)\s+(\d+\.\d+)/;

			# we want the zero byte latencies
			($1 != 0) and next;
			$data{"ave_$testname"} = $3;
			$status = 'FOUNDDATA';	

			# that's all the data we need, reset the state machine
			$parse_state = 0;
		}
		elsif ($parse_state == 3) {
			($l =~ /\#bytes\s+\#repetitions/) and next;
			$l =~ /\s+(\d+)\s+(\d+)\s+(\d+\.\d+)\s+(\d+\.\d+)\s+(\d+\.\d+)\s+(\d+\.\d+)/;

			if ($1 == 8192) {
				$data{"ave_$testname"} = $6;
				$status = 'FOUNDDATA';	
			}

			# we ideally want the bandwidth at 4MB (just seems the most stressful...)
			($1 != 4194304) and next;

			$data{"ave_$testname"} = $6;
			$status = 'FOUNDDATA';	

			# that's all the data we need, reset the state machine
			$parse_state = 0;
		}
		elsif ($parse_state == 4) {
			($l =~ /\#bytes\s+\#repetitions/) and next;
			($l =~ /Attention, msg size/) and next;
			$l =~ /\s+(\d+)\s+(\d+)\s+(\d+\.\d+)\s+(\d+\.\d+)\s+(\d+\.\d+)/;

			# we want the elapsed time at 8K message sizes, that's arbirarily picked
			($1 != 8192) and next;
			$data{"ave_$testname"} = $5;
			$status = 'FOUNDDATA';	

			(defined $main::DEBUG and $main::DEBUG > 2) and print
				"DEBUG:imb: parse_state=$parse_state $status\n";

			# that's all the data we need, reset the state machine
			$parse_state = 0;
		}
		elsif ($parse_state == 5) {
			($l =~ /\#repetitions/) and next;
			$l =~ /\s+(\d+)\s+(\d+\.\d+)\s+(\d+\.\d+)\s+(\d+\.\d+)/;

			$data{"ave_$testname"} = $4;
			$status = 'FOUNDDATA';	

			# that's all the data we need, reset the state machine
			$parse_state = 0;
		}
	}

	# check to see if all the tests that were supposed to run generated
    # an output table
    foreach my $k (keys %tests) {
    	if ($tests{$k} == 0) {
			$error = 1;
            
			(defined $main::DEBUG and $main::DEBUG > 2) and print
				"DEBUG:imb: MISSINGDATA for $k $tests{$k}\n";
        }
    }
    
	if ($status =~ /FOUNDDATA/ and !$error) {
		$data{'STATUS'} = "PASSED";
	}
	else {
		$data{'STATUS'} = "ERROR($status)";
        defined $main::diagnose and main::print_job_err($fileid,'ERROR',$status);
	}

	return \%data;
}

=item B<name()> - Return the name of the parse module

For example: output_parse::xhpl

=cut

sub name {
	my $self = shift;
	return $self->{NAME};
}

=item B<files_list()> - Return the ordered array of file(s) we
want to parse

For example: ['STDOUT','STDERR'] or  ['STDOUT','hpccoutf.txt']

STDOUT and STDERR are keywords and not actual filenames. The
keywords are groked by the output parsing core code and handled
appropriately.

=cut

sub file_list {
	my $self = shift;

	return \@{$self->{FILE_LIST}};
}


=item B<alias_spec()> - Return a regex specification of benchmark names
we will parse in addition to our module name, i.e. 'imb'

This routine is optional.

=cut

sub alias_spec {
	my $self = shift;

	return "(imball|imbcust)";
}

=item B<metric_units()> - Return a hash with a mapping of the
metrics this module returns to a string specifying the units
of the metric

=cut

sub metric_units {
	my $self = shift;

	return \%{$self->{METRIC_UNITS}};
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

	# save our name so callers can ask us
	$self->{NAME} = $package;
	$self->{SHORTNAME} = $shortpackage;

	# this is a KEY array... see the file_list method above for
	# more info
	@{$self->{FILE_LIST}} = qw/STDOUT/;

	# this is a KEY array... see the metric_units method above for
	# more info
	%{$self->{METRIC_UNITS}} = (
		'ave_PingPong' => 'us',
		'ave_PingPing' => 'us',
		'ave_Sendrecv' => 'MB/s',
		'ave_Exchange' => 'MB/s',
		'ave_Allreduce' => 'us',
		'ave_Reduce' => 'us',
		'ave_Reduce_scatter' => 'us',
		'ave_Allgather' => 'us',
		'ave_Allgatherv' => 'us',
		'ave_Alltoall' => 'us',
		'ave_Bcast' => 'us',
		'ave_Barrier' => 'us',
	);
	
	return 1;
}


1;
