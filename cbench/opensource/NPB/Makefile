include ../../make.def

INSTALLDIR = $(bin_dir)
#INSTALLTARGETS = $(shell find bin -type f)

SRC=NPB3.2.1
TAR=$(SRC).tar.gz

MPIDIR=$(SRC)/NPB3.2-MPI
SERDIR=$(SRC)/NPB3.2-SER


all: serial mpi

$(MPIDIR)/config/suitedef:
	./gen_suitedef --nasdir $(MPIDIR) --maxprocs $(MAXPROCS)

$(SERDIR)/config/suitedef:
	./gen_suitedef --nasdir $(SERDIR) --serial

serial: $(SRC) .patched $(SERDIR)/config/suitedef
	@ if [ ! -d bin ] ; then mkdir bin ; fi
	cd $(SERDIR)/config; ln -sf ../../../make.def .
	$(MAKE) CC=$(PLAINCC) F77=$(PLAINF77) F90=$(PLAINF90) -C $(SERDIR) suite

mpi: $(SRC) .patched $(MPIDIR)/config/suitedef
	@ if [ ! -d bin ] ; then mkdir bin ; fi
	cd $(MPIDIR)/config; ln -sf ../../../make.def .
	$(MAKE) -C $(MPIDIR) suite

clean: cleanserial cleanmpi

cleanserial:
	$(MAKE) -C $(SERDIR) clean; /bin/rm -f $(SERDIR)/config/suite.def
	/bin/rm -f bin/*.[ABCDSW]

cleanmpi:
	$(MAKE) -C $(MPIDIR) clean; /bin/rm -f $(MPIDIR)/config/suite.def
	/bin/rm -f bin/*.[ABCDSW].*

install uninstall:
	$(do-$@)

$(SRC): $(TAR)
	[ -d $(SRC) ] || tar zxf $(TAR) && rm -f .patched
	ln -sf $(SRC) src

.patched:
	for patch in *.patch ; do \
           cd $(SRC); \
           echo "Applying patch [ $$patch ]"; \
           patch -p1 <../$$patch; \
           cd - ; \
        done
	-touch .patched

distclean:
	/bin/rm -rf $(SRC) .patched src bin
