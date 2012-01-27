include ../../make.def

VERSION=4.2.1
SRC=memtester-$(VERSION)
TAR=$(SRC).tar.gz
URL="http://pyropus.ca/software/memtester/old-versions/$(TAR)"

INSTALLDIR = $(hwtest_bin_dir)
INSTALLTARGETS = src/memtester

default: all

download: $(TAR)

$(TAR):
	$(call download-curl-url,$(URL))

$(SRC): $(TAR)
	$(call untar-src,$(TAR))
	ln -sf $(SRC) src

all: $(SRC)
	$(MAKE) -C src $@

clean:
	$(MAKE) -C src clean

distclean:
	-/bin/rm -rf $(SRC) $(TAR) src

install uninstall:
	$(do-$@)

