include ../../make.def

SRC=miranda_io-1.0.1
TAR=$(SRC).tar.gz
URL=https://computing.llnl.gov/code/sio/tarballs/$(TAR)

EXECS = miranda_io
INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

all normal: $(EXECS)

$(EXECS): $(SRC)
	$(F90) $(FFLAGS) -o miranda_io $(SRC)/miranda_io.f90

$(SRC): $(TAR)
	-[ ! -d $(SRC) ] && tar zxf $(TAR)
	-[ ! -s src ] && ln -sf $(SRC) src
	patch -p0 < fix-compile.patch
	patch -p0 < swl.patch
	@touch $(SRC) src

$(TAR) download:
	$(call download-url,$(URL))
	-[ -e $(TAR) ] && touch $(TAR)

clean:
	/bin/rm -f $(EXECS)

distclean: clean
	/bin/rm -rf $(SRC) $(TAR) src

install uninstall:
	$(do-$@)
