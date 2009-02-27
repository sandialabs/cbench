include ../../make.def

INSTALLDIR = $(hwtest_bin_dir)
INSTALLTARGETS = src/cachebench/cachebench

default: all

all:
	$(MAKE) -C src reconfig
	$(MAKE) -C src linux-mpich
	$(MAKE) -C src/cachebench cachebench

clean:
	$(MAKE) -C src clean; $(MAKE) -C src/cachebench clobber

distclean: clean

install uninstall:
	$(do-$@)
