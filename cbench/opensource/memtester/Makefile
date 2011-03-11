include ../../make.def

VERSION=4.2.1
SRC=memtester-$(VERSION)
TAR=$(SRC).tar.gz
URL="http://pyropus.ca/software/memtester/old-versions/$(TAR)"

INSTALLDIR = $(hwtest_bin_dir)
INSTALLTARGETS = src/memtester

default: all

$(TAR):
	@echo "Downloading $(TAR)"
	$(call download-url,$(URL))

$(SRC): $(TAR)
	@echo "Extracting $(SRC)"
	@if [ -f $(TAR) ] ; then tar -zxf $(TAR); \
	else \
		echo "$(TAR) not found, download manually or set http_proxy env variable?"; \
		exit 1; \
	fi
	ln -sf $(SRC) src

all: $(SRC)
	$(MAKE) -C src $@

clean:
	$(MAKE) -C src clean

distclean:
	-/bin/rm -rf $(SRC) $(TAR) src

install uninstall:
	$(do-$@)

