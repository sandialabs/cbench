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
	$(call download-curl-url,$(URL))

$(SRC): $(TAR)
	$(call untar-src,$(TAR))
	ln -sf $@ src
	@[ -d $@ ] && touch $@ && ln -sf $@ src

$(EXEC): $(SRC)
	make -C src CC=$(PLAINCC) CXX=$(PLAINCXX) > .make-$(SRC) 2>&1
	@[ -x $@ ] && touch $@
#	cd src/; export CC=$(CC); export F77=$(F77); export MPILIB=$(MPICH)/lib; if [ ! -f config.status ]; then ./configure; fi; make

clean:
	make -C src clean > .make-$(SRC) 2>&1

distclean: 
	/bin/rm -rf $(SRC) $(TAR) src .make-$(SRC)

install uninstall: 
	$(do-$@)
