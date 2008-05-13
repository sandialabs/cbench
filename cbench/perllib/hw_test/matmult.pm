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


package hw_test::matmult;

use strict;

my $package = __PACKAGE__;
my ($shortpackage) = $package =~ /hw_test::(\S+)/;

=head1 NAME

matmult

Cbench hw_test module that uses the LLNL Matmult test
of compute stress on UNIX-like operating systems.


=cut

###############################################################
#
# Public methods
#


sub new {
	my $type = shift;
	my $self = {};
	bless $self, $type;
	$self->_init(@_) or return;

	return $self;
}


sub run {
	my $self = shift;
	
	# grab the output file handle for the object
	my $ofh = $self->{outhandle};

	# path to the binaries
	my $path = "$main::bench_test/$main::TESTBINPATH";
	
	# how many cpu cores
	my $numcores = main::linux_num_cpus();

	my @buf = ();
	my $cmd = '';

	# the following parameters and logic were ported from the runit.{mmc,mmf}
	# scripts of matmult
	my $maxrep = 5000;
	my @size_list = qw/128 256 1024 2048/;

	if (defined $main::SMALL) {
		main::debug_print(1,"DEBUG:$shortpackage\.run() doing SMALL mode runs\n");
		$maxrep = 1000;
	}

	foreach my $size (@size_list) {
		my $nrep = int ($maxrep / $size);
		($nrep == 0) and $nrep = 1;

		# matmult is an OpenMP dependent test fundamentally, so max out the
		# thread load
		$ENV{'OMP_NUM_THREADS'} = $numcores;
		foreach my $mm (qw/mmc mmf/) {
			# check for the binary since it isn't necessarily built
			if (! -x "$path/$mm") {
				print $ofh "ERROR: $path/$mm does not exist\n";
				next;
			}

			$cmd = "$path/$mm $numcores $nrep $size $size $size";
			main::debug_print(2,"DEBUG: $shortpackage\.run() cmd=$cmd\n");

			my $date = `/bin/date`;
			chomp $date;
			print $ofh "====> $mm, $date\n";

			my $start = time;
			main::run_single_process("$cmd 2>&1",\@buf);
			my $end = time;

			print $ofh @buf;
			# clear out the buffer for the next binary/iteration
			$#buf = -1;

			# compute number of minutes the stress run took
			my $delta = ($end - $start) / 60;
			print $ofh "$mm Elapsed Time: $delta minutes\n";
		}
	}
}


sub parse {
	my $self = shift;
	my $bufref = shift;

	my %data;
	my $binary = "NADA";
	my $speedup_tmp = "NADA";
	my $elapsed_tmp = 'NADA';
	my $elapsed = 0;

	# parse the buffer
	# NOTE: matmult runs a mmc and mmf binary with multiple
	#       parameter combinations
	foreach (@$bufref) {
		if (/====> (\S+),/) {
			if ($binary eq 'NADA') {
				$binary = $1;
			}
			else {
				my $key = "$self->{SHORTNAME}_$binary\_speedup";
				($speedup_tmp ne 'NADA') and $data{$key} = main::max($data{$key},$speedup_tmp);
				($speedup_tmp ne 'NADA') and $elapsed += $elapsed_tmp;

				# reset stuff for output from the next binary
				$speedup_tmp = 'NADA';
				$elapsed_tmp = 'NADA';
				$binary = $1;
			}
		}
		elsif (/Average speedup is\s+(\S+)/) {
			$speedup_tmp = $1;
			main::debug_print(3,"DEBUG:$shortpackage\.parse() $binary speedup $speedup_tmp");
		}
		elsif (/Elapsed Time:\s+(\d+\.\d+)\s+minutes/) {
			$elapsed_tmp = $1;
			main::debug_print(3,"DEBUG:$shortpackage\.parse() $binary elapsed $elapsed_tmp");
		}

		main::debug_print(3,"DEBUG:$shortpackage\.parse() process $binary, $elapsed");
	}

	# build the hash with the data retrieved from parsing
	my $key = "$self->{SHORTNAME}_$binary\_speedup";
	($speedup_tmp ne 'NADA') and $data{$key} = main::max($data{$key},$speedup_tmp);
	my $key = "$self->{SHORTNAME}_elapsed";
	($speedup_tmp ne 'NADA') and $elapsed += $elapsed_tmp;
	$data{$key} = $elapsed;
	#my $key = "$self->{SHORTNAME}\_failed";
	#$data{$key} = $failed;

	return \%data;
}


sub name {
	my $self = shift;
	return $self->{NAME};
}


sub test_class {
	my $self = shift;
	return $self->{TEST_CLASS};
}



###############################################################
#
# "Private" methods
#

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
