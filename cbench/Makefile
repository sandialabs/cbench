# vim: syntax=make tabstop=4
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

include make.def

HW_TESTS = fpck ctcs memtester streams nodeperf llcbench stress matmult stride stab 

OPEN_TESTS = b_eff mpi_latency mpi_hello mpi_hello_ordered mpi_overhead \
             mpi_routecheck ior_bench iozone bonnie++ rotate HPLlinpack \
             IMB osutests lanl stab perftest mpi_examples mpi_slowcpu \
             mpi_malloc mpi_tokensmash phloem HPLinpack2 \
             fdtree simul mdtest miranda_io graph500

OPENEXTRAS_TESTS = NPB hpcc

# all of these live in the opensource/ area
HWTEST_SUBDIRS     = $(addprefix opensource/, $(HW_TESTS))
OPEN_SUBDIRS       = $(addprefix opensource/, $(OPEN_TESTS))
OPENEXTRAS_SUBDIRS = $(addprefix opensource/, $(OPENEXTRAS_TESTS))

SUBDIRS = $(OPEN_SUBDIRS) $(HWTEST_SUBDIRS)

ifndef BINIDENT
  BINDIR = bin
else
  BINDIR = bin.$(BINIDENT)
endif
  DEST = $(BENCH_TEST)/$(BINDIR)

# by default build all the standard opensource set of binaries
default: open hwtest

open: $(OPEN_SUBDIRS)
	$(do-open-subdirs)

extras: $(OPENEXTRAS_SUBDIRS)
	$(do-openextras-subdirs)

iotest:
	$(MAKE) -C opensource/iozone
	$(MAKE) -C opensource/ior_bench
	$(MAKE) -C opensource/iozone install
	$(MAKE) -C opensource/ior_bench install

hwtest: $(HWTEST_SUBDIRS)
	$(do-hwtest-subdirs) 
	$(MAKE) -C opensource/iozone
	$(MAKE) -C opensource/iozone install

# target used to build only the binaries needed to fully utilize the 
# NODEHWTEST testset, i.e. compile only what is needed for node-level testing
nodehwtest: hwtest standalone standaloneinstall
	$(MAKE) -C opensource/NPB serial
	$(call do-hwtest-subdirs,install)

# BEGIN STANDALONE 
cbenchstandalonedir: openmpi
	@sbin/install_cbenchtest --testtop
	$(MAKE) -C opensource/ompi install
	$(MAKE) -C opensource/GotoBLAS2 distclean
	$(MAKE) -C opensource/GotoBLAS2 install
	$(call do-hwtest-subdirs,distclean)
	$(do-hwtest-subdirs)
	$(call do-hwtest-subdirs,install)
	$(MAKE) -C opensource/hdparm distclean
	$(MAKE) -C opensource/hdparm
	$(MAKE) -C opensource/hdparm install
	$(MAKE) -C opensource/iozone distclean
	$(MAKE) -C opensource/iozone
	$(MAKE) -C opensource/iozone install
	$(MAKE) -C opensource/dt distclean
	$(MAKE) -C opensource/dt
	$(MAKE) -C opensource/dt install
	$(MAKE) -C opensource/bonnie++ distclean
	$(MAKE) -C opensource/bonnie++
	$(MAKE) -C opensource/bonnie++ install
	$(MAKE) -C opensource/ior_bench distclean
	$(MAKE) -C opensource/ior_bench
	$(MAKE) -C opensource/ior_bench install
	$(MAKE) -C opensource/fdtree install
	$(MAKE) -C opensource/mdtest distclean
	$(MAKE) -C opensource/mdtest
	$(MAKE) -C opensource/mdtest install
	$(MAKE) -C opensource/lanl distclean
	$(MAKE) -C opensource/lanl 
	$(MAKE) -C opensource/lanl install
	$(MAKE) -C opensource/HPLlinpack distclean
	$(MAKE) -C opensource/HPLlinpack
	$(MAKE) -C opensource/HPLlinpack install
	$(MAKE) -C opensource/hpcc distclean
	$(MAKE) -C opensource/hpcc
	$(MAKE) -C opensource/hpcc install
	$(MAKE) -C opensource/HPLinpack2 distclean
	$(MAKE) -C opensource/HPLinpack2
	$(MAKE) -C opensource/HPLinpack2 install
	$(MAKE) -C opensource/NPB distclean
	$(MAKE) -C opensource/NPB serial
	$(MAKE) -C opensource/NPB install
	$(MAKE) -C opensource/graph500 
	$(MAKE) -C opensource/graph500 install
	$(do-compiled-bin-installs-binident)
	@sbin/install_cbenchtest --allsets --bindir $(BINDIR)

openmpi: 
	$(MAKE) -C opensource/ompi distclean
	$(MAKE) -C opensource/ompi 

# more to test the flow of new tests to integrate... can remove later -cdm
cbenchstandalonedirbins:
	$(do-compiled-bin-installs-binident)
	@sbin/install_cbenchtest --allsets --bindir $(BINDIR)
#
# END STANDALONE 

# targets to compile the Cbench standalone MPI
mpich: opensource/mpich/lib/libmpich.a
opensource/mpich/lib/libmpich.a:
	$(MAKE) -C opensource/mpich clean
	$(MAKE) -C opensource/mpich

# compile the standalone MPI binaries used in the NODEHWTEST testset
standalone: mpich
	$(MAKE) CBENCH_STANDALONE=yes -C opensource/lanl clean
	$(MAKE) CBENCH_STANDALONE=yes -C opensource/lanl
	$(MAKE) CBENCH_STANDALONE=yes -C opensource/HPLlinpack clean
	$(MAKE) CBENCH_STANDALONE=yes -C opensource/HPLlinpack
	$(MAKE) CBENCH_STANDALONE=yes -C opensource/HPLinpack2 clean
	$(MAKE) CBENCH_STANDALONE=yes -C opensource/HPLinpack2
	$(MAKE) CBENCH_STANDALONE=yes -C opensource/hpcc clean
	$(MAKE) CBENCH_STANDALONE=yes -C opensource/hpcc
# install the standalone MPI binaries used in the NODEHWTEST testset
standaloneinstall:
	$(MAKE) -C opensource/mpich install
	$(MAKE) CBENCH_STANDALONE=yes -C opensource/lanl install
	$(MAKE) CBENCH_STANDALONE=yes -C opensource/HPLlinpack install
	$(MAKE) CBENCH_STANDALONE=yes -C opensource/HPLinpack2 install
	$(MAKE) CBENCH_STANDALONE=yes -C opensource/hpcc install

# compile and install HPCC
hpcc:
	$(MAKE) -C opensource/hpcc normal
	$(MAKE) -C opensource/hpcc install

# install all the default compiled binaries
install uninstall: default
	$(do-subdirs)

# install the Cbench testing tree (CBENCHTEST variable must be set)
installtests itests:
	@sbin/install_cbenchtest --testtop
	$(do-compiled-bin-installs-binident)
	@sbin/install_cbenchtest --allsets --bindir $(BINDIR)

# just install compiled binaries in to the Cbench testing tree
installcbenchbins: install
	@sbin/install_cbenchtest --testtop
	$(do-compiled-bin-installs-binident)

# update the core parts of the Cbench testing tree
update: 
	@sbin/install_cbenchtest --testtop

download:
	$(do-subdirs)

# compile and install LAMMPS
lammps: install 
	$(do-lammps-install)
	@sbin/install_cbenchtest --testset lammps
	$(BENCH_HOME)/openapps/lammps/install_lammps

#cbench-tests-rpm: 

clean:
	$(do-subdirs)
	$(MAKE) -C opensource/hpcc clean
	rm -rf bin VERSION.snapshot cbench_snapshot_*.tar.gz cbench_release_*.tar.gz

distclean: clean
	$(MAKE) -C opensource/hpcc distclean
	$(MAKE) -C opensource/NPB distclean
	$(MAKE) -C opensource/mpich distclean
	$(do-subdirs)

# targets used for making tarballs
release: distclean
	export cbench_rev=`grep Version VERSION | awk -F ': ' '{print $$2}'`; tar cfvz cbench_release_$${cbench_rev}\.tar.gz * --exclude cbench*.tar.gz --exclude CVS --exclude .svn --exclude restricted --exclude *.o --exclude bin --exclude openapps

repo_release: distclean
	export cbench_rev=`grep Version VERSION | awk -F ': ' '{print $$2}'`; export cbench_tag=`svn info . | grep Revision | awk '{print $$2}'`; tar cfvz cbench_snapshot_$${cbench_rev}-$${cbench_tag}\.tar.gz * --exclude cbench*.tar.gz --exclude CVS --exclude .svn --exclude restricted --exclude *.o --exclude bin --exclude openapps

apps_release: distclean
	export cbench_rev=`grep Version VERSION | awk -F ': ' '{print $$2}'`; tar cfvz cbench_app_release_$$cbench_rev\.tar.gz * --exclude cbench*.tar.gz --exclude CVS --exclude .svn --exclude restricted --exclude *.o --exclude bin 

restricted_release: distclean
	export cbench_rev=`grep Version VERSION | awk -F ': ' '{print $$2}'`; tar cfvz cbench_restricted_release_$$cbench_rev\.tar.gz * --exclude cbench*.tar.gz --exclude CVS --exclude .svn --exclude *.o --exclude bin 

#
# backdoor shorcuts for developers...
#

# compile most everything in the main tree except nodehwtest stuff
domostall: 
	$(MAKE) distclean
	$(MAKE) install
	$(MAKE) hpcc

# compile everything in the main tree (not openapps for example)!
doitall: 
	$(MAKE) distclean
	$(MAKE) install
	$(MAKE) hpcc
	$(MAKE) nodehwtest
