include ../../make.def

VERSION=3.2.3
SRC=IMB_$(VERSION)
TAR=$(SRC).tgz
URL="http://software.intel.com/sites/products/mpi-benchmarks/$(TAR)"

EXEC = src/src/IMB-MPI1-$(VERSION)

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = src/src/IMB-MPI1-$(VERSION)

default: $(EXEC)

download: $(TAR)

$(TAR):
	$(call download-curl-url,$(URL))

$(SRC): $(TAR)
	$(call untar-src,$(TAR))
	mv imb_$(VERSION) $(SRC)
	ln -sf $@ src
	@[ -d $@ ] && touch $@

.make-$(SRC): $(SRC)
	@echo "Compiling $(SRC)"
	$(MAKE) -C src/src CC=$(CC) > .make-$(SRC) 2>&1
	@touch $@

$(EXEC): .make-$(SRC)
	[ -x src/src/IMB-MPI1 ] && cp -f src/src/IMB-MPI1 src/src/IMB-MPI1-$(VERSION)

clean:
	$(MAKE) -C src/src clean > .make-$(SRC) 2>&1
	rm -f .make-$(SRC) 

distclean:
	rm -fr .make-$(SRC) src $(TAR) $(SRC)

install uninstall: default
	$(do-$@)

