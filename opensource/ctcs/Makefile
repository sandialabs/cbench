include ../../make.def

INSTALLDIR = $(hwtest_bin_dir)
INSTALLTARGETS = src/runin/memtst

default: all

all:
	$(MAKE) -C src CC=$(PLAINCC) CLINKER='$(PLAINCC) $(LDFLAGS)' CFLAGS='$(CFLAGS)' MEMTST_CFLAGS='$(CFLAGS) $(NOUNROLL_FLAGS)' $@

distclean clean:
	$(MAKE) -C src dist-clean
	-/bin/rm -rf src/runin/bin

install uninstall:
	$(do-$@)
