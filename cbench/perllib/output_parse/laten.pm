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


package output_parse::laten;

use strict;

my $package = __PACKAGE__;
my ($shortpackage) = $package =~ /output_parse::(\S+)/;

=head1 NAME

laten

Cbench parse module responsible for parsing output files generated
by the Presta 'laten' benchmark
.

=cut

###############################################################
#
# Public methods
#


=head1 PUBLIC METHODS

=over 4

=item B<new()> - Create a new parse object

  $obj = output_parse::laten->new();

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

  $obj = output_parse::laten->new();
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

	# NOTE: The laten output is a little more of a pain to parse since
	#       it uses a table of latencies at different process counts.
	#       We are interested in the latency for the process count
	#       corresponding to our job size. This should be the last value
	#       in the table.  In any event, we use a little state machine
	#       controlled by $parse_state.
    my $status = 'NOTSTARTED';
	my $parse_state = 0;
    foreach my $l (@{$txtbuf}) {
        ($l =~ /CBENCH NOTICE/) and $status = 'NOTICE';
        ($l =~ /MPI.*Bidir.*latency/) and $status = 'STARTED';
		($l =~ /Processes.*/) and $parse_state = 1;
		($parse_state > 0) or next;
		($l =~ /Test Parameters/) and $parse_state = 2 and
			$status = 'COMPLETED';


		# ignore table divider lines
		($l =~ /\-\-\-\-\-\-\-\-\-/) and next;
		
		if ($parse_state = 1) {
			# parsing the table values
			if ($l =~ /\s+(\d+)\s+(\d+\.\d+)/) {
				($1 == $main::np) and $data{'latency'} = $2;
			}
		}
	}

	if ($status =~ /COMPLETED/) {
		$data{'STATUS'} = "PASSED";
	}
	elsif ($status =~ /NOTICE/) {
		# this means the job was not an error, but did not
		# run because the benchmark does not support running
		# on an odd number of processors
		$data{'STATUS'} = $status;
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
		'latency' => 'us',
	);
	
	return 1;
}


1;
