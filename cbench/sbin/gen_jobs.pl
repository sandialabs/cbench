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


# Top-level script in the Cbench test set tree to recursively call
# the *_gen_jobs.pl script for each test set.

# need to know where everything cbench lives!
use lib ($ENV{CBENCHOME} ? $ENV{CBENCHOME} : "$ENV{HOME}\/cbench");
$BENCH_HOME = $ENV{CBENCHOME} ? $ENV{CBENCHOME} :
    "$ENV{HOME}\/cbench";
require "cbench.pl";

use Getopt::Long;

# pass_through lets "non options" stay in ARGV w/o raising an error
Getopt::Long::Configure("pass_through");

GetOptions( 'debug:i' => \$DEBUG,
            'dryrun' => \$dryrun,
            'help' => \$help,
);


if (defined $help) {
        usage();
        exit;
}
$bench_test = get_bench_test();

$pwd = `pwd`;
chomp $pwd;

for $f (`/bin/ls -1`) {
	chomp $f;
	next if ($f eq "." || $f eq "..");
	(! -d $f) and next;
	# ignore certain directories because they are not test sets or are
	# uniquely behaving test sets like nodehwtest
	($f =~ /nodehwtest|perllib|bin|sbin|mpich|tools|templates/) and next;
	
	print "Generating jobs in ". uc($f) . " test set\n";
	
	chdir $f;
	
	# build the command line
	$cmd = "./$f\_gen_jobs.pl ";
    (defined $DEBUG) and $cmd .= " --debug $DEBUG ";
    $cmd .= join(' ',@ARGV);
	
	($DEBUG) and print "DEBUG: cmd=$cmd\n";
	($dryrun) and print "$cmd\n";
	system($cmd) unless $dryrun;
	
	chdir $pwd;
}

sub usage {
    print "USAGE: $0 \n";
    print "Cbench script to generate jobs in the all test sets\n".
          "   --dryrun         Show what would be done without doing it\n".
          "   --debug <level>  turn on debugging at the specified level\n\n".
          "   All other command-line options are passed on to the\n".
          "   specific *_gen_jobs.pl scripts\n";
}

