# Makefile for HPLinpack 2.0 for Cbench

include ../../make.def

export CFLAGS

SRC=hpl-2.0
TAR=$(SRC).tar.gz
URL=http://www.netlib.org/benchmark/hpl/$(TAR)
MPICH_CH=ch_shmem
HPL2=$(CBENCHOME)/opensource/HPLinpack2

ifeq ($(CBENCH_STANDALONE),yes)
  $(warning ================ STANDALONE MPI COMPILE)
  EXECS = /src/bin/cbench/xhpl2.$(MPICH_CH)
  default_tgt := local
  MPIHOME = $(CBENCHOME)/opensource/mpich
  MPIBINNAME = bin
else
  # NORMAL MPI COMPILE
  EXECS = src/bin/cbench/xhpl2
  default_tgt := linpack
endif

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

default: $(default_tgt) 

all: linpack clean local

linpack: $(SRC) xerbla
	$(MAKE) -C $(SRC) arch=cbench
	[ -e src/bin/cbench/xhpl ] && /bin/mv src/bin/cbench/xhpl src/bin/cbench/xhpl2

$(SRC): $(TAR)
	-[ ! -d $(SRC) ] && tar xzf $(TAR)
	ln -sf $(SRC) src
	ln -sf $(HPL2)/Make.cbench $(HPL2)/$(SRC)/Make.cbench
	touch $(SRC)

xerbla: 
	$(CC) $(OPTFLAGS) -c xerbla.c

# This is a special compile that uses a local Cbench version of MPICH
# using the $(MPICH_CH) device.  We use this to compile a version of 
# hpl that will run just on a single node.
local: linpack
	/bin/mv -f $(HPL2)/src/bin/cbench/xhpl2 $(HPL2)/src/bin/cbench/xhpl2.$(MPICH_CH)

$(TAR):
	$(call download-url,$(URL))
	-[ -e $(TAR) ] && touch $(TAR)

clean:
	-[ -d $(SRC) ] && $(MAKE) -C $(SRC) arch=cbench clean
	/bin/rm -rf $(HPL2)/src/lib/* $(HPL2)/src/bin/*
	/bin/rm -f xerbla.o

distclean:
	/bin/rm -rf $(SRC) xerbla.o src $(TAR)

install uninstall:
	$(do-$@)
