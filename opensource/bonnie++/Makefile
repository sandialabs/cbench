# Makefile for bonnie++ for Cbench

include ../../make.def

SRC=bonnie++-1.03d
TAR=$(SRC).tgz
URL=http://www.coker.com.au/bonnie++/$(TAR)

EXECS = $(SRC)/bonnie++

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

default: $(SRC)
	cd src/; export CC=$(CC); export F77=$(F77); export MPILIB=$(MPICH)/lib; if [ ! -f config.status ]; then ./configure; fi; make

$(SRC) src: $(TAR)
	if [ ! -d bonnie++-1.03d ]; then tar -zxvf bonnie++-1.03d.tgz; fi
	ln -sf $(SRC) src
	touch $(SRC)

$(TAR) download:
	$(call download-url,$(URL))
	-[ -e $(TAR) ] && touch $(TAR)

clean:
	-cd $(SRC); make realclean

distclean: clean
	-/bin/rm -rf $(SRC) $(TAR) src

install uninstall:
	$(do-$@)
