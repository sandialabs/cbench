include ../../make.def

VERSION=2
SRC=UsingMPI$(VERSION)
TAR=$(SRC).tar.gz
URL="http://ftp.mcs.anl.gov/pub/mpi/usingmpi2/$(TAR)"

EXECS = src/stress
INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

default: .make-$(SRC)

download: $(TAR)

$(TAR):
	@echo "Downloading $(TAR)"
	$(call download-sf-url,$(URL))

$(SRC): $(TAR)
	@echo "Extracting $(SRC)"
	@if [ -f $(TAR) ] ; then tar -xxf $(TAR); \
	else \
		echo "$(TAR) not found, download manually or set http_proxy env variable?"; \
		exit 1; \
	fi
	ln -sf $(SRC) src

.configure-$(SRC): $(SRC)
	@echo "Configuring $(SRC)"
	cd src && MPICC=$(CC) MPIF77=$(F77) CC=$(PLAINCC) F77=$(PLAINF77) CFLAGS='$(CFLAGS)' FFLAGS='$(FFLAGS)' ./configure  --with-lammpi=$(MPIHOME) > ../.configure-$(SRC) 2>&1
	@touch $@

.make-$(SRC): .configure-$(SRC)
	@echo "Compiling $(SRC)"
	cd src && make > ../.make-$(SRC) 2>&1
	@touch $@

clean:
	@if [ -f src/Makefile ] ; then $(MAKE) -C src clean; fi
	rm -f .make-$(SRC) .configure-$(SRC)

distclean:
	rm -fr .make-$(SRC) .configure-$(SRC) src $(TAR) $(SRC)

install: .make-$(SRC)
	@echo "Installing $(SRC)"
	cd src && make install >> ../.make-$(SRC) 2>&1
	@for progs in dshbak  pdcp  pdsh  rpdcp; do \
		ln -sf $(CBENCHTEST)/$(SRC)/bin/$$progs $(CBENCHTEST)/sbin/$$progs; \
	done

