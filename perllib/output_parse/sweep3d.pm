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

package output_parse::sweep3d;

use strict;

my $package = __PACKAGE__;
my ($shortpackage) = $package =~ /output_parse::(\S+)/;

=head1 NAME

sweep3d

Cbench parse module responsible for parsing output files generated
by the SWEEP3D 2.2b Pipelined Wavefront with Line-Recursion simulation

=cut

##################################################################
#
# Public methods
#

=head1 PUBLIC METHODS

=over 4

=item B<new()> - Create a new parse object

    $obj = output_parse::npb->new();

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

    $obj = output_parse::lammps->new();
    .
    read in an output file int @buf
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
    # a string identifying the file(s) from which we are parsing
    # output; used mainly for printing things out
    my $fileid = shift;
    # should be one or more buffer references passed in;
    # they are ordered according to our very own FILE_LIST
    # array (see file_list() below)
    my @bufrefs = @_;

    my %data;

    # process all the lines in the STDOUT buffer
    my $txtbuf = $bufrefs[0];
    my $numlines = scalar @$txtbuf;

    my $status = 'NOTSTARTED';
    my $found_endrecord = 0;

    my $mem_per_domain = 0;
    my $domains = 1;
#use Data::Dumper;
#print STDERR Dumper(\$txtbuf);
    foreach my $l (@{$txtbuf}) {

		($l =~ /NOT BUILT/) and
			$status = 'NOTBUILT';

        # first line output by SWEEP3D
		($l =~ /SWEEP3D - Method 5 - Pipelined Wavefront with Line-Recursion/) and $status = 'STARTED';

        # all processes print a 'FORTRAN STOP' message before exiting
		($l =~ /Elapsed time/) and $found_endrecord = 1 and 
			$status = 'SUCCESSFUL';
		#	    $status = 'COMPLETED';

        #
		# this is where the data parsing takes place
        #
		if ($l =~ /memory usage per domain:\s+(.*)/) {
            $mem_per_domain = $1;
		}

		# capture CPU time
		if ($l =~ /CPU\s+time was:\s+(.*)/) {
			$data{"cpu_time"} = $1;
		}

		# capture CPU grind time
		if ($l =~ /CPU grind time:\s+(.*)/) {
			$data{'cpu_grind_time'} = $1;
		}

        # save # of domains for calculating overall memory usage
        if ($l =~ /\s+(.*)domains.*/) {
            $domains = $1;
        }

        #
        # end data parsing
        #

    }

    if ($status =~ /SUCCESSFUL/) {
		$data{'STATUS'} = "PASSED";
    }
    elsif ($status =~ /UNSUCCESSFUL/) {
		$data{'STATUS'} = "FAILED VERIFICATION";
    }
    else {
	$data{'STATUS'} = "ERROR($status)";
	defined $main::diagnose and main::print_job_err($fileid,'ERROR',$status);
    }
#	$found_endrecord or next;

    # calculate the total amount of memory used
    $data{'memory_total'} = $mem_per_domain * $domains;

#use Data::Dumper;
#print STDERR Dumper(\%data);
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

For example: ['STDOUT','STDERR'] or ['STDOUT','hpccoutf.txt']

STDOUT and STDERR are keywords and not actual filenames. The 
keywords are groked by the output parsing core code and handled
appropriately.

=cut

sub file_list {
    my $self = shift;

    return \@{$self->{FILE_LIST}};
}

=item B<alias_spec()> - Return a regex specification of benchmark
names we will parse in addition to our module name, i.e. 'sweep3d'

This routine is optional.

=cut

sub alias_spec {
    my $self = shift;

    return "150std|long";
}

=item B<metric_units()> - Return a hash with a mapping of the
metrics this module returns to a string specifying the units
of the metric

=cut

sub metric_units {
    my $self = shift;

    return \%{$self->{METRIC_UNITS}};
}

################################################################
#
# "Private" methods
#

=head1 PRIVATE METHODS

=over 4

=item B<_init> - parse arguments for B<new()>

Parse the arguments passed to B<new()> and set the appropriate
object variables and such.

=cut

sub _init {
    my $self = shift;

    # save our name so callers can ask us
    $self->{NAME} = $package;
    $self->{SHORTNAME} = $shortpackage;

    # this is a KEY array... see the file_list method for more
    # info
    @{$self->{FILE_LIST}} = qw/STDOUT/;

    # this is a KEY array... see the metric_units method for more
    # info
    %{$self->{METRIC_UNITS}} = (
	'memory' => 'MB',
	'cpu_time' => 'seconds',
	'cpu_grind_time' => 'seconds',
    );

    return 1;
}

1;
