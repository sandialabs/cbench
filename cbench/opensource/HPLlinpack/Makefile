include ../../make.def

MPICH_CH=ch_shmem

ifeq ($(CBENCH_STANDALONE),yes)
  $(warning ================ STANDALONE MPI COMPILE)
  EXECS = hpl/bin/cbench/xhpl.$(MPICH_CH)
  default_tgt := local
  MPIHOME = $(CBENCHOME)/opensource/mpich
  MPIBINNAME = bin
else
  #$(warning ================ NORMAL MPI COMPILE)
  EXECS = hpl/bin/cbench/xhpl
  default_tgt := linpack
endif

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

.SUFFIXES: .so.gz .so.gz

%.gz: %.so.gz
	gunzip $<

%.so.gz: %.so
	gzip $<

ZIPPED_LIBS = $(wildcard *.gz)
UNZIPPED_LIBS = $(patsubst %.so.gz,%.gz,$(ZIPPED_LIBS))

CLEAN_LIBS_UNZIPPED = $(wildcard *.so)
CLEAN_LIBS_ZIPPED = $(patsubst %.so,%.so.gz,$(CLEAN_LIBS_UNZIPPED))


default: linpack-size $(default_tgt)

linpack-size: linpack-size.c
	$(CC) $(OPTFLAGS) -o $@ linpack-size.c -lm

xerbla:
	$(CC) $(OPTFLAGS) -c xerbla.c
	#$(F77) $(OPTFLAGS) -c xerbla.f

linpack: xerbla
	$(MAKE) -C hpl

local: linpack
	/bin/mv -f hpl/bin/cbench/xhpl hpl/bin/cbench/xhpl.ch_shmem

clean:
	/bin/rm -f linpack-size	
	$(MAKE) -C hpl make.def
	$(MAKE) -C hpl clean_arch_all
	/bin/rm -rf hpl/lib hpl/bin

install uninstall:
	$(do-$@)
