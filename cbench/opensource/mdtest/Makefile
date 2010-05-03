include ../../make.def

SRC=mdtest-1.7.4
TAR=$(SRC).tar.gz
URL=http://downloads.sourceforge.net/project/mdtest/mdtest/$(SRC)/$(TAR)

EXECS = $(SRC)/mdtest
INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

all normal: $(EXECS)

$(EXECS): $(SRC)
	$(MAKE) CC.Linux=$(CC) -C $(SRC)

$(SRC): $(TAR)
	-[ ! -d $(SRC) ] && tar zxf $(TAR)
	-[ ! -s src ] && ln -sf $(SRC) src

$(TAR) download:
	$(call download-url,$(URL))
	-[ -e $(TAR) ] && touch $(TAR)

clean:
	-[ -d $(SRC) ] && $(MAKE) -C $(SRC) clean || :

distclean:
	/bin/rm -rf $(SRC) $(TAR) src

install uninstall:
	$(do-$@)
