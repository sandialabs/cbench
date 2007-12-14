include ../../make.def

SRC=stride
TAR=$(SRC).tar.gz
URL=ftp://ftp.llnl.gov/outgoing/$(TAR)

EXECS = $(addprefix src/, cachec.Opt cachedot.Opt cache.Opt strid3c.Opt strid3.Opt striddot.Opt vecopc.Opt vecop.Opt)
INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

normal: $(SRC)
	$(MAKE) -C src -f Makefile.cbench

$(SRC): $(TAR)
	[ ! -d $(SRC) ] && tar zxf $(TAR)
	ln -sf $(SRC)/src src
	cd src && ln -sf ../../Makefile.cbench
	$(MAKE) -C src -f Makefile.cbench clean

$(TAR):
	wget $(URL)
	-[ -e $(TAR) ] && touch $(TAR)

clean:
	[ -d $(SRC) ] && $(MAKE) -C src -f Makefile.cbench clean || :

distclean:
	/bin/rm -rf $(SRC) $(TAR) src

install uninstall:
	$(do-$@)
