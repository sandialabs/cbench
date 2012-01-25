include ../../make.def

SRC=hpcc-1.4.1
TAR=$(SRC).tar.gz
HPL=$(SRC)/hpl
MPICH_CH=ch_shmem
URL=http://icl.cs.utk.edu/projectsfiles/hpcc/download/$(TAR)

# hpcc.ch_shmem won't exist for the normal case, so make install will print an error
ifeq ($(CBENCH_STANDALONE),yes)
  $(warning ================ STANDALONE MPI COMPILE)
  EXECS = $(SRC)/hpcc.$(MPICH_CH)
  default_tgt := local
  MPIHOME = $(CBENCHOME)/opensource/mpich
  MPIBINNAME = bin
else
  #$(warning ================ NORMAL MPI COMPILE)
  EXECS = $(SRC)/hpcc
  default_tgt := normal
endif
INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

default: $(default_tgt)

all: normal clean local

# the normal compile using make.def stuff
normal: $(SRC) .patched make.def xerbla
	cd $(SRC); ln -sf $(CBENCHOME)/make.def
	cd $(HPL)/lib/arch/build; ln -sf $(CBENCHOME)/make.def make.def
	$(MAKE) -C $(SRC) arch=cbench

debug: $(SRC) .patched .debugpatched make.def xerbla
	cd $(SRC); ln -sf $(CBENCHOME)/make.def
	cd $(HPL)/lib/arch/build; ln -sf $(CBENCHOME)/make.def make.def
	$(MAKE) -C $(SRC) arch=cbench

# This is a special compile that uses a local Cbench version of MPICH
# using the $(MPICH_CH) device. We use this to compile a version of HPCC that
# will run on just a single node.
local: normal
	/bin/mv -f $(SRC)/hpcc $(SRC)/hpcc.$(MPICH_CH)

xerbla:
	$(CC) $(OPTFLAGS) -c xerbla.c

clean: 
	-[ -d $(SRC) ] && $(MAKE) -C $(SRC) arch=cbench clean
	/bin/rm -f $(SRC)/hpcc.$(MPICH_CH)
	-[ -d $(HPL) ] && $(MAKE) -C $(HPL) arch=cbench clean_arch_all
	/bin/rm -rf $(HPL)/bin
	-find ./$(SRC) -name make.def -print -exec /bin/rm -f {} \;
	-[ -d $(HPL) ] && cd $(HPL) && /bin/rm -f Make.cbench
	/bin/rm -f xerbla.o

distclean:
	/bin/rm -rf $(SRC) xerbla.o .patched src .debugpatched $(TAR)

make.def: $(SRC)
	[ -d $(HPL) ] && cd $(HPL) && ln -sf $(CBENCHOME)/opensource/hpcc/Make.cbench .
	[ -d $(HPL) ] && cd $(HPL) && ln -sf $(CBENCHOME)/make.def .
	[ -d $(SRC) ] && cd $(SRC); ln -sf $(CBENCHOME)/make.def

$(SRC): $(TAR)
	[ ! -d $(SRC) ] && tar zxf $(TAR) && rm -f .patched .debugpatched
	ln -sf $(SRC) src

$(TAR):
	$(call download-url,$(URL))
	-[ -e $(TAR) ] && touch $(TAR)

download: $(TAR)

.patched:
	for patch in *.patch ; do \
           cd $(SRC); \
           echo "Applying patch [ $$patch ]"; \
           patch -p1 <../$$patch; \
           cd - ; \
        done
	-touch .patched

.debugpatched:
	for patch in *.debugpatch ; do \
           cd $(SRC); \
           echo "Applying patch [ $$patch ]"; \
           patch -p1 <../$$patch; \
           cd - ; \
        done
	-touch .debugpatched

install uninstall:
	$(do-$@)
