#!/usr/bin/perl

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
# vim: syntax=perl tabstop=4

# need to know where everything cbench lives!
BEGIN {
    die "Please define CBENCHOME!\n" if !defined($ENV{CBENCHOME});
}
use lib $ENV{CBENCHOME};
require "cbench.pl";
$CBENCHOME = $BENCH_HOME = $ENV{CBENCHOME};

# add Cbench perl library to the Perl search path
use lib "$ENV{CBENCHOME}\/perllib";

use Getopt::Long;

# this is a list of all the supported Cbench testsets that can
# be installed
my $core_testsets = "bandwidth linpack npb rotate nodehwtest mpioverhead latency collective io iosanity hpcc mpisanity shakedown";
my $testsets = $core_testsets;

my $gazebo_config = 'cbench_config';

GetOptions( 'ident=s' => \$ident,
			'debug:i' => \$DEBUG,
			'testsets=s' => \$testsets,
			'gazebohome|gazhome=s' => \$gazebo_home,
			'gazeboconfig|gazconfig=s' => \$gazebo_config, 
			'minprocs=i' => \$minprocs,
			'maxprocs=i' => \$maxprocs,
			'procs=i' => \$procs,
);

# clean out existing submit_config if there is one
system("/bin/rm -f $gazebo_home/submit_configs/$gazebo_config 1>/dev/null 2>&1");

# go to the top of the Cbench testing tree
my $cbenchtest = get_bench_test();
chdir $cbenchtest;


my @testset_list = split(',',$testsets);
debug_print(1,"DEBUG: testsets= $testsets");
foreach my $set (@testset_list) {
	chdir $set;

	print "Hooking Cbench ".uc($set)." testset into Gazebo\n";
	# --gazebo --testset latency --maxprocs 16 --gazhome /home/jbogden/tlcc/gazebo+cbench/Gazebo --gazeboconfig config_cbench
	my $cmd = "./$set\_gen_jobs.pl --gazebo --gazhome $gazebo_home --gazeboconfig $gazebo_config";
	(defined $minprocs) and $cmd .= " --minprocs $minprocs";
	(defined $maxprocs) and $cmd .= " --maxprocs $maxprocs";
	(defined $procs) and $cmd .= " --procs $procs";

	debug_print(1,"DEBUG: cmd= $cmd");
	system("$cmd");

	chdir $cbenchtest;
}

print "Wrote Gazebo submit config here: $gazebo_home/submit_configs/$gazebo_config\n";
