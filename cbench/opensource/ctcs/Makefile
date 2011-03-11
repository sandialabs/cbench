include ../../make.def

VERSION = 1.3.1pre1
CTCS = ctcs-1.3.1-pre1
TAR = $(CTCS).tgz

URL="http://sourceforge.net/projects/va-ctcs/files/va-ctcs/$(VERSION)/$(TAR)"

$(TAR) download:
	$(call download-sf-url,$(URL),$(TAR))
	-[ -e $(TAR) ] && touch $(TAR)

INSTALLDIR = $(hwtest_bin_dir)
INSTALLTARGETS = src/runin/memtst

default: all

all:
	$(MAKE) -C src CC=$(PLAINCC) CLINKER='$(PLAINCC) $(LDFLAGS)' CFLAGS='$(CFLAGS)' MEMTST_CFLAGS='$(CFLAGS) $(NOUNROLL_FLAGS)' $@

distclean clean:
	-[ -d $(CTCS) ] && rm -fr $(CTCS)
	-[ -f $(TAR) ] && rm -f $(TAR)
	$(MAKE) -C src dist-clean
	-/bin/rm -rf src/runin/bin

install uninstall:
	$(do-$@)
