# Makefile for Graph500 for Cbench

# NOTE: as of June 2011 I haven't had any luck compiling graph500
#       with PGI 10.0+ compilers. GCC and Intel seem to work fine.

include $(CBENCHOME)/make.def

export CFLAGS

#SRC=graph500-1.2
SRC=graph500-2.1.4
TAR=$(SRC).tar.bz2
URL=http://www.graph500.org/$(TAR)

# this block was used to build graph500 from the alternate source tree on 
# Github... your mileage may vary
#
#SRC=rudolph9-Graph500-b2841d6
#TAR=b2841d66c7b308a4e9711df259fb829d225df048.tgz
#URL=https://github.com/rudolph9/Graph500/tarball/b2841d66c7b308a4e9711df259fb829d225df048
#wget --no-check-certificate https://github.com/rudolph9/Graph500/tarball/b2841d66c7b308a4e9711df259fb829d225df048 -O b2841d66c7b308a4e9711df259fb829d225df048.tgz

GRAPH500=$(CBENCHOME)/opensource/graph500

# NORMAL MPI COMPILE
EXECS = src/mpi/graph500_mpi_one_sided src/mpi/graph500_mpi_simple
default_tgt := mpi

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

default: $(default_tgt) 

download: $(TAR)

$(TAR):
	$(call download-sf-url,$(URL))

$(SRC): $(TAR)
	$(call untar-src, $(TAR))
	ln -sf $(SRC) src
	touch $(SRC)

all: mpi 

mpi: $(SRC) src/make.inc clean
	$(MAKE) -C $(SRC)/mpi MPICC=$(CC) CC=$(PLAINCC) CFLAGS="-Drestrict=__restrict__ -DNDEBUG -DGRAPH_GENERATOR_MPI $(CFLAGS)" LDFLAGS=$(LDFLAGS) 
#	$(MAKE) -C $(SRC) MPICC=$(CC) CC=$(PLAINCC) make-edgelist

src/make.inc:
	ln -sf $(GRAPH500)/make.inc-cbench $(GRAPH500)/src/make.inc

clean:
	-[ -d $(SRC) ] && $(MAKE) -C $(SRC) clean

distclean:
	/bin/rm -rf $(SRC) src $(TAR) .patched

install uninstall:
	$(do-$@)
