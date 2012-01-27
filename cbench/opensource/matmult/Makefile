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

default: .make-$(SRC)

download: $(TAR)

$(TAR):
	$(call download-curl-url,$(URL))

$(SRC): $(TAR)
	$(call untar-src,$(TAR))
	ln -sf $(SRC)/matmult src
	cd src; mv Makefile Makefile.orig;ln -s ../../Makefile.cbench Makefile

.patched:
	for patch in *.patch ; do \
           cd $(SRC); \
           echo "Applying patch [ $$patch ]"; \
           patch -p1 <../$$patch; \
           cd - ; \
        done
	-touch .patched

.make-$(SRC): $(SRC) second.o .patched
	$(MAKE) FC=$(PLAINF90) CC=$(PLAINCC) CFLAGS="$(CFLAGS) -DDGEMM -DMXMD16 -DLINUX" \
		FFLAGS="$(FFLAGS) -DLINUX" LDFLAGS="$(LDFLAGS)" -C src $@

clean:
	[ -d $(SRC) ] && $(MAKE) -C src clean
	/bin/rm -f second.o 

distclean:
	/bin/rm -rf $(SRC) $(TAR) src second.o .patched

install uninstall:
	$(do-$@)
