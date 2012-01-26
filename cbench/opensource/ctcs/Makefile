include ../../make.def

VERSION = 1.3.1pre1
SRC = ctcs-1.3.1-pre1
TAR = $(SRC).tgz
URL="http://sourceforge.net/projects/va-ctcs/files/va-ctcs/$(VERSION)/$(TAR)"

INSTALLDIR = $(hwtest_bin_dir)
INSTALLTARGETS = src/runin/memtst

default: all

download: $(TAR)

$(TAR):
	@echo "Downloading $(TAR)"
	$(call download-sf-url,$(URL),$(TAR))

$(SRC): $(TAR)
	@echo "Extracting $(SRC)"
	@if [ -f $(TAR) ] ; then tar -zxf $(TAR); \
	else \
		echo "$(TAR) not found, download manually or set http_proxy env variable?"; \
		exit 1; \
	fi
	mv ctcs-1.3.1pre1 $(SRC)
	ln -sf $(SRC) src

all: $(SRC)
	$(MAKE) -C src CC=$(PLAINCC) CLINKER='$(PLAINCC) $(LDFLAGS)' CFLAGS='$(CFLAGS)' MEMTST_CFLAGS='$(CFLAGS) $(NOUNROLL_FLAGS)' $@

clean:
	$(MAKE) -C src dist-clean
	/bin/rm -rf src/runin/bin

distclean:
	rm -fr $(SRC) $(TAR) src


install: default
	$(do-$@)

uninstall: 
	$(do-$@)
