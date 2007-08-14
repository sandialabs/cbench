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

# Script to install a skeleton Cbench testset using the generic
# templates

# need to know where everything cbench lives!
$BENCH_HOME = $ENV{CBENCHOME} ? $ENV{CBENCHOME} :
    "$ENV{HOME}\/cbench";

unshift @INC, "$BENCH_HOME";
require "cbench.pl";

use Getopt::Long;

GetOptions(
	'name=s' => \$name,
);

(defined $name) or die "--name parameter required";

# get the BENCH_TEST path from make.def
$bench_test = get_bench_test();
$testset = $name;
$testset_path = "$bench_test/$testset";
$DEBUG and print "DEBUG: $bench_test $testset_path\n";

# build the directory tree
mk_test_dir($testset,$bench_test);

# setup the list of templates to install and do it
my $tmp = `/bin/ls -1 $BENCH_HOME/templates/$testset\*.in 2>&1`;
if ($tmp =~ /$testset/ and $tmp !~ /no such/i) {
	print "Found job templates for a testset named \'$testset\', installing them...\n";
	@files = ("templates/$testset\*.in" );
	rsync_filelist(\@files,$testset_path);
}
else {
	print "Installing skeleton hello world job template into the testset as $testset\_hello.in\n";
	system("rsync $BENCH_HOME/templates/skeleton_hello.in $testset_path/$testset\_hello.in");
}

# install templated utility scripts
install_util_script("tools/start_jobs_jobdirs.pl","$testset_path/$testset\_start_jobs.pl",
	"$testset");
install_util_script("tools/gen_jobs_generic.pl","$testset_path/$testset\_gen_jobs.pl",
	"$testset");
install_util_script("tools/output_parse_generic.pl","$testset_path/$testset\_output_parse.pl",
	"$testset");
