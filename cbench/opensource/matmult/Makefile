include ../../make.def

SRC=MATMULT
SRCTAR=MATMULT_GPL
TAR=$(SRCTAR).tar.gz
# no llnl site for this tarball yet
URL=http://cbench.org/common/tarballs/$(TAR)

EXECS = $(addprefix src/, mmc mmf) 
INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

# turn on OpenMP support
COMMON_FLAGS += $(OPENMP_ENABLE)

# matmult needs both BLAS and LAPACK linkages, so depeneding on what
# blas lib we are using, we might also need lapack
WHICHBLAS = $(findstring goto,$(BLASLIB))
ifeq ($(WHICHBLAS),goto)
  LDFLAGS=../../second.o $(LAPACKLIB) $(BLASLIB) -lm
else
  LDFLAGS=$(BLASLIB) -lm
endif

ifeq ($(COMPILER),gcc)
 #LDFLAGS += -lg2c
endif

# this is for the second.c implicit rule to get the CC we want
CC=$(PLAINCC) 

all: $(SRC) second.o .patched
	$(MAKE) FC=$(PLAINF90) CC=$(PLAINCC) CFLAGS="$(CFLAGS) -DDGEMM -DMXMD16 -DLINUX" \
		FFLAGS="$(FFLAGS) -DLINUX" LDFLAGS="$(LDFLAGS)" -C src $@

$(SRC): $(TAR)
	[ ! -d $(SRC) ] && tar zxf $(TAR)
	touch $(SRC)
	ln -s $(SRC)/matmult src
	cd src; mv Makefile Makefile.orig;ln -s ../../Makefile.cbench Makefile

$(TAR) download:
	$(call download-url,$(URL))
	-[ -e $(TAR) ] && touch $(TAR)

.patched:
	for patch in *.patch ; do \
           cd $(SRC); \
           echo "Applying patch [ $$patch ]"; \
           patch -p1 <../$$patch; \
           cd - ; \
        done
	-touch .patched

clean:
	[ -d $(SRC) ] && $(MAKE) -C src clean
	/bin/rm -f second.o .patched

distclean:
	/bin/rm -rf $(SRC) $(TAR) src second.o .patched

install uninstall:
	$(do-$@)
