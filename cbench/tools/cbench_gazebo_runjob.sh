#!/bin/bash
#
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
# vim: syntax=sh tabstop=4

# script to be executed inside the Gazebo "setUpandRun" that wraps the 
# actual running of Cbench jobs. It depends on the Gazebo test_exec config
# files being generated by the Cbench+Gazebo hook script, cbench_gazebo_submit_config.pl

#env
echo " "
echo " "
echo " "
echo "---------- Setup Cbench Environment -----------------------------------------------"
CBENCH_PPN=$GZ_PESPERNODE"ppn"
CBENCH_MATCH=$CBENCH_JOB-$CBENCH_PPN
echo "CBENCH_PPN=$CBENCH_PPN"
echo "CBENCH_MATCH=$CBENCH_MATCH"

# NOTE: some  CBENCH_  environment vars will be set for us by the Gazebo script that
#       called us based on the gazebo test config file

env | egrep 'CBENCH|NPES|JOBID|GAZ|GZ'
echo " "
echo " "
echo " "
# head over to the Cbench testing tree and setup the environment for using
# that tree
cd $CBENCHTEST
. cbench-init.sh
cd $CBENCH_TESTSET

echo "---------- Start Cbench Job -------------------------------------------------------"
./$CBENCH_TESTSET\_start_jobs.pl --interactive --procs $GZ_NPES --match $CBENCH_MATCH --ident $CBENCH_TESTIDENT --echooutput --gazebo 2>&1 | tee $RUNHOME/temp.start
grep -q "ERROR: No job started" $RUNHOME/temp.start
if [ $? -eq 0 -o ! -f "$RUNHOME/temp.start" ]; then
	echo "ERROR: Cbench did not seem to find a job matching these criteria:"
	echo "  number of processors (procs/GZ_NPES): $GZ_NPES"
	echo "  processes/cores per node (ppn/GZ_PESPERNODE): $GZ_PESPERNODE"
	echo "  Cbench --match: $CBENCH_MATCH"
	echo "  Cbench --ident: $CBENCH_TESTIDENT"
fi
echo " "
echo " "
echo " "
echo "---------- End of Cbench Job ------------------------------------------------------"

echo " "
echo " "
echo " "
echo "---------- Parse Cbench Job Output -------------------------------------------------------"
# find the jobid identifying the cbench run
jobid=`grep "Cbench jobid:" $RUNHOME/temp.start | awk '{print $3'}`
echo "Cbench jobid was $jobid"
echo " "
echo " "
echo " "
if [ -z "$jobid" ]; then
	echo "FAIL: Cbench jobid was not found. Cbench probably did not find a job to start."
	exit 1
else
	./$CBENCH_TESTSET\_output_parse.pl --procs $NPES --ident $CBENCH_TESTIDENT --match $CBENCH_MATCH --jobid $jobid --gazebo
fi

exit 0
