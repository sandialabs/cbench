# Makefile for Graph500 1.2 for Cbench

include ../../make.def

export CFLAGS

SRC=graph500-1.2
TAR=$(SRC).tar.bz2
URL=http://www.graph500.org/$(TAR)
GRAPH500=$(CBENCHOME)/opensource/graph500
#HPL2=$(CBENCHOME)/opensource/HPLinpack2

# NORMAL MPI COMPILE
EXECS = src/mpi/graph500_mpi_one_sided src/mpi/graph500_mpi_simple
default_tgt := mpi

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

default: $(default_tgt) 

all: mpi 

mpi: $(SRC)
	$(MAKE) -C $(SRC)

$(SRC): $(TAR)
	-[ ! -d $(SRC) ] && tar xjf $(TAR)
	ln -sf $(SRC) src
	ln -sf $(GRAPH500)/make.inc-cbench $(GRAPH500)/src/make.inc
	touch $(SRC)

$(TAR):
	$(call download-url,$(URL))
	-[ -e $(TAR) ] && touch $(TAR)

download: $(TAR)

clean:
	-[ -d $(SRC) ] && $(MAKE) -C $(SRC) clean

distclean:
	/bin/rm -rf $(SRC) src $(TAR)

install uninstall:
	$(do-$@)
