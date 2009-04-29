include $(CBENCHOME)/make.def

# https://asc.llnl.gov/computing_resources/purple/archive/benchmarks/sppm/sppm1.1.tar
SRC=sppm
TAR=sppm1.1.tar
URL=https://asc.llnl.gov/computing_resources/purple/archive/benchmarks/sppm/$(TAR)


EXECS = $(SRC)/sppm
INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

all:

HIDDENall: $(SRC)
	make CC=$(CC) -C $(SRC) veryclean
	$(MAKE) CC=$(CC) -C $(SRC)

$(SRC) src: $(TAR)
	[ ! -d $(SRC) ] && tar xf $(TAR)
	ln -sf $(SRC) src
	cd $(SRC); /bin/rm -f Makefile.include
	cd $(SRC); ln -sf ../Makefile.include .
	touch $(SRC)

$(TAR) download:
	$(call download-url,$(URL))
	-[ -e $(TAR) ] && touch $(TAR)

clean:
	make CC=$(CC) -C $(SRC) veryclean

distclean:
	/bin/rm -rf $(TAR) $(SRC) src

install uninstall:
	$(do-$@)

include $(CBENCHOME)/make.inc
