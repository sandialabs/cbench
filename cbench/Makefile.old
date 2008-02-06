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

HWTEST_SUBDIRS = opensource/fpck opensource/ctcs opensource/memtester \
                 opensource/streams opensource/nodeperf opensource/llcbench \
				 opensource/stress

OPEN_SUBDIRS = opensource/b_eff opensource/mpi_latency \
               opensource/mpi_hello opensource/mpi_hello_ordered opensource/mpi_overhead \
               opensource/mpi_routecheck opensource/mpi_tokensmash \
               opensource/PMB2.2.1 opensource/presta1.2 \
               opensource/ior_bench opensource/iozone opensource/rotate \
               opensource/HPLlinpack opensource/IMB opensource/osutests \
               opensource/lanl  opensource/stab \
               opensource/perftest opensource/mpi_examples opensource/mpi_slowcpu

OPENEXTRAS_SUBDIRS = opensource/NPB opensource/hpcc

SUBDIRS = $(OPEN_SUBDIRS) $(HWTEST_SUBDIRS)

# these are the test sets we try to install into the Cbench test tree
CONFIGURED_TESTSETS = bandwidth linpack npb rotate nodehwtest mpioverhead latency collective io iosanity hpcc mpisanity

# by default build all the standard opensource set of binaries as well as the basic
# hardware testing binaries
default: open hwtest

open: $(OPEN_SUBDIRS)
	$(do-open-subdirs)

extras: $(OPENEXTRAS_SUBDIRS)
	$(do-openextras-subdirs)

hwtest: $(HWTEST_SUBDIRS)
	$(do-hwtest-subdirs)
	$(MAKE) -C opensource/NPB serial
	$(MAKE) -C opensource/iozone
	$(MAKE) -C opensource/iozone install

# target used to build only the binaries needed to fully utilize the 
# NODEHWTEST testset, i.e. compile only what is needed for node-level testing
nodehwtest: hwtest standalone standaloneinstall
	$(call do-hwtest-subdirs,install)

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
	$(MAKE) CBENCH_STANDALONE=yes -C opensource/hpcc clean
	$(MAKE) CBENCH_STANDALONE=yes -C opensource/hpcc
# install the standalone MPI binaries used in the NODEHWTEST testset
standaloneinstall:
	$(MAKE) CBENCH_STANDALONE=yes -C opensource/lanl install
	$(MAKE) CBENCH_STANDALONE=yes -C opensource/HPLlinpack install
	$(MAKE) CBENCH_STANDALONE=yes -C opensource/hpcc install

# compile and install HPCC
hpcc:
	$(MAKE) -C opensource/hpcc normal
	$(MAKE) -C opensource/hpcc install

# install all the default compiled binaries
install uninstall: default
	$(do-subdirs)

# install the Cbench testing tree (CBENCHTEST variable must be set)
installtests: install
	@sbin/install_testtop
	$(do-compiled-bin-installs)
	$(do-testset-installs)

# just install compiled binaries in to the Cbench testing tree
installcbenchbins: install
	@sbin/install_testtop
	$(do-compiled-bin-installs)

# a convenience target to compile the larger set of commonly used binaries
# and install the Cbench testing tree
installcbench: domostall
	@sbin/install_testtop
	$(do-compiled-bin-installs)
	$(do-testset-installs)

# just update the Cbench testing tree scripts
updatecbench: 
	@sbin/install_testtop
	@sbin/update_util_scripts

# compile and install LAMMPS
lammps: install 
	$(do-lammps-install)

#cbench-tests-rpm: 

clean:
	$(do-subdirs)
	$(MAKE) -C opensource/hpcc clean
	rm -rf bin VERSION.snapshot cbench_snapshot_*.tar.gz cbench_release_*.tar.gz

distclean: clean
	$(MAKE) -C opensource/hpcc distclean
	$(MAKE) -C opensource/NPB clean
	$(MAKE) -C opensource/mpich clean
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

no_exports_restricted_release: distclean
	export cbench_rev=`grep Version VERSION | awk -F ': ' '{print $$2}'`; tar cfvz cbench_restricted_release_exportsok_$$cbench_rev\.tar.gz * --exclude cbench*.tar.gz --exclude CVS --exclude .svn --exclude *.o --exclude bin --exclude EXPORT-RESTRICTED 

#
# backdoor shorcuts for developers...
#

# compile most everything
domostall: 
	$(MAKE) distclean
	$(MAKE) install
	$(MAKE) hpcc

# compile everything!
doitall: 
	$(MAKE) distclean
	$(MAKE) install
	$(MAKE) hpcc
	$(MAKE) standalone
	$(MAKE) standaloneinstall

itests:
	@sbin/install_testtop
	$(do-compiled-bin-installs)
	$(do-testset-installs)

ifndef BINIDENT
  DEST = $(BENCH_TEST)/bin
else
  DEST=$(BENCH_TEST)/bin.$(BINIDENT)
endif

itests-with-ident:
	@echo sbin/install_testtop
	$(do-compiled-bin-installs-binident)
	$(do-testset-installs)
