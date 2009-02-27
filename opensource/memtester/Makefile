include ../../make.def

INSTALLDIR = $(hwtest_bin_dir)
INSTALLTARGETS = src/memtester

default: all

all:
	$(MAKE) -C src $@

clean:
	$(MAKE) -C src clean

distclean: clean

install uninstall:
	$(do-$@)
