include ../../make.def

export CFLAGS

SRC=HPCCG-0.3
TAR=$(SRC).tar.gz
URL=http://www.cs.sandia.gov/~maherou/$(TAR)

EXECS = $(SRC)/test_HPCCG
INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

normal: $(SRC)
	cd $(SRC) && ln -sf ../Makefile.cbench Makefile.cbench
	$(MAKE) -C $(SRC) -f Makefile.cbench

$(SRC): $(TAR)
	[ ! -d $(SRC) ] && tar zxf $(TAR)
	ln -sf $(SRC) src

$(TAR):
	wget $(URL)
	-[ -e $(TAR) ] && touch $(TAR)

clean:
	[ -d $(SRC) ] && $(MAKE) -C $(SRC) -f Makefile.cbench clean || :

distclean:
	/bin/rm -rf $(SRC) $(TAR) src

install uninstall:
	$(do-$@)
