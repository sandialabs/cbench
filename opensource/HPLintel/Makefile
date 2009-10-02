# Makefile for HPLinpack 2.0 for Cbench


include ../../make.def

export CFLAGS
SRC=l_lpk_p_10.2.0.004
UNTARSRC=linpack_10.2.0
TAR=$(SRC).tar.gz
URL=https://registrationcenter.intel.com/irc_nas/1466/${TAR}
MPICH_CH=ch_shmem
HPL2=$(CBENCHOME)/opensource/HPLintel

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
	$(MAKE) -C $(UNTARSRC)/benchmarks/mp_linpack arch=cbench
#	[ -e src/bin/cbench/xhpl ] && /bin/mv src/bin/cbench/xhpl src/bin/cbench/xhplintel

$(SRC): $(TAR)
	-[ ! -d $(UNTARSRC) ] && tar xzf $(TAR)
	ln -sf $(UNTARSRC) src
	ln -sf $(HPL2)/Make.cbench $(HPL2)/$(UNTARSRC)/benchmarks/mp_linpack/Make.cbench
	touch $(UNTARSRC)

xerbla: 
	$(CC) $(OPTFLAGS) -c xerbla.c

# This is a special compile that uses a local Cbench version of MPICH
# using the $(MPICH_CH) device.  We use this to compile a version of 
# hpl that will run just on a single node.
local: linpack
	/bin/mv -f $(HPL2)/src/bin/cbench/xhplintel $(HPL2)/src/bin/cbench/xhplintel.$(MPICH_CH)

$(TAR):
	$(call download-url,$(URL))
	-[ -e $(TAR) ] && touch $(TAR)

clean:
	-[ -d $(UNTARSRC) ] && $(MAKE) -C $(UNTARSRC)/benchmarks/mp_linpack clean_arch_all arch=cbench
	/bin/rm -rf $(HPL2)/src/lib/* $(HPL2)/src/bin/*
	/bin/rm -f xerbla.o

distclean:
	/bin/rm -rf $(UNTARSRC) xerbla.o src $(TAR)

install uninstall:
	$(do-$@)
