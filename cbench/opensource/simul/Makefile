include ../../make.def

SRC=simul-1.14
TAR=$(SRC).tar.gz
URL=https://computing.llnl.gov/code/sio/tarballs/$(TAR)

EXECS = simul
INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

all normal: $(EXECS)

$(EXECS): $(SRC)
	$(CC) $(CFLAGS) -o simul $(SRC)/simul.c

$(SRC): $(TAR)
	-[ ! -d $(SRC) ] && tar zxf $(TAR)
	-[ ! -s src ] && ln -sf $(SRC) src
	@touch $(SRC) src

$(TAR) download:
	$(call download-url,$(URL))
	-[ -e $(TAR) ] && touch $(TAR)

clean:
	/bin/rm -f simul

distclean: clean
	/bin/rm -rf $(SRC) $(TAR) src

install uninstall:
	$(do-$@)
