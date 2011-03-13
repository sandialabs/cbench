# Makefile for bonnie++ for Cbench

include ../../make.def

#VERSION=1.03e
VERSION=1.96
SRC=bonnie++-$(VERSION)
TAR=$(SRC).tgz

#URL=http://www.coker.com.au/bonnie++/$(TAR)
URL="http://www.coker.com.au/bonnie++/experimental/$(TAR)"

EXECS = $(SRC)/bonnie++

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

default: $(SRC)
	make -C src CC=$(PLAINCC) CXX=$(PLAINCXX)
#	cd src/; export CC=$(CC); export F77=$(F77); export MPILIB=$(MPICH)/lib; if [ ! -f config.status ]; then ./configure; fi; make

$(TAR):
	@echo "Downloading $(TAR)"
	$(call download-url,$(URL))

$(SRC): $(TAR)
	@echo "Extracting $(SRC)"
	@if [ -f $(TAR) ] ; then tar -zxf $(TAR); \
	else \
		echo "$(TAR) not found, download manually or set http_proxy env variable?"; \
		exit 1; \
	fi
	ln -sf $(SRC) src

clean:
	make -C src realclean 

distclean: 
	-/bin/rm -rf $(SRC) $(TAR) src

install uninstall:
	$(do-$@)

