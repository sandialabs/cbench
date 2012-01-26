# Makefile for bonnie++ for Cbench

include ../../make.def

#VERSION=1.03e
VERSION=1.96
SRC=bonnie++-$(VERSION)
TAR=$(SRC).tgz

#URL=http://www.coker.com.au/bonnie++/$(TAR)
URL="http://www.coker.com.au/bonnie++/experimental/$(TAR)"

EXEC = $(SRC)/bonnie++

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXEC)

default: $(EXEC)

download: $(TAR)

$(TAR):
	$(call download-sf-url,$(URL))

$(SRC): $(TAR)
	$(call untar-src, $(TAR))
	ln -sf $(SRC) src

$(EXEC): $(SRC)
	make -C src CC=$(PLAINCC) CXX=$(PLAINCXX) > .make-$(SRC) 2>&1
#	cd src/; export CC=$(CC); export F77=$(F77); export MPILIB=$(MPICH)/lib; if [ ! -f config.status ]; then ./configure; fi; make

clean:
	make -C src clean 

distclean: 
	/bin/rm -rf $(SRC) $(TAR) src .make-$(SRC)

install: default
	$(do-$@)

uninstall: 
	$(do-$@)
