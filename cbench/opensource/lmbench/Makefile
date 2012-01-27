include ../../make.def

SRC=lmbench3
TAR=$(SRC).tar.gz
URL="http://www.bitmover.com/lmbench/$(TAR)"

EXECS = lat_mem_rd

INSTALLDIR = $(hwtest_bin_dir)
INSTALLTARGETS = src/bin/x86_64-linux-gnu/$(EXECS)

default: all

download: $(TAR)

$(TAR):
	$(call download-curl-url,$(URL))

$(SRC): $(TAR)
	$(call untar-src,$(TAR))
	ln -sf $(SRC) src
	patch -p1 -d src < lmbench3.cbench.patch

all: $(SRC)
	$(MAKE) CC=$(CC) CFLAGS='$(CFLAGS)' -C src

clean:
	$(MAKE) -C src clean

distclean:
	-/bin/rm -rf $(SRC) $(TAR) src

install uninstall:
	$(do-$@)

