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

package parse_filter::torque;

use strict;

my $package = __PACKAGE__;
my ($shortpackage) = $package =~ /parse_filter::(\S+)/;

# This module contains custom parse filters that will optionally be
# employed by the Cbench output parsers when looking at job output.
# With these filters, the user can dynamically tell the output parsers
# to look for user-defined errors. These filters are only used when
# the --customparse parameter is given to the *_output_parse.pl scripts.
#
# Each filter is composed of a key/value pair.  The key is is a Perl
# regular expression including capturing. The value is a string that will
# be used to identify when an occurrence of the regex is found. The value
# can include the $1,$2,... etc variables that would be captured by
# the Perl regular expression
#
# The %parse_filter_include hash in cluster.def controls which parse
# filter modules are used by the output parsing engine.
#
# These parse filters are for Torque
our %parse_filters = (
	'PBS: job killed: node (\d+) \((\S+)\) requested job die, code (\d+)' =>
		'PBS JOB DIE REQUEST, node $2 code=$3',

	'=>> PBS: job killed: node (\d+) \((\S+)\) requested job terminate, \'EOF\' \(code (\d+)\) - internal or network failure attempting to communicate with sister MOM\'s' =>
		'PBS JOB KILLED request from $2 code=$3 due to communication failure, check status of nodes in this job',

	'=>> PBS: job killed: node (\d+) \((\S+)\) requested job die, \'EOF\' \(code (\d+)\) - internal or network failure attempting to communicate with sister MOM\'s' =>
		'PBS JOB DIE REQUEST due to communication failure to node $2 code=$3',

	'PBS: job killed: walltime.*exceeded limit' =>
		'PBS JOB WALLTIME EXCEEDED',
);


1;

