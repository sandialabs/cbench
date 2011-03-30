include ../../make.def

SRC=lmbench3
TAR=$(SRC).tar.gz
URL="http://www.bitmover.com/lmbench/$(TAR)"

EXECS = lat_mem_rd

INSTALLDIR = $(hwtest_bin_dir)
INSTALLTARGETS = src/bin/x86_64-linux-gnu/$(EXECS)

default: all

$(TAR):
	@echo "Downloading $(TAR)"
	$(call download-url,$(URL))

download: $(TAR)

$(SRC): $(TAR)
	@echo "Extracting $(SRC)"
	@if [ -f $(TAR) ] ; then tar -zxf $(TAR); \
	else \
		echo "$(TAR) not found, download manually or set http_proxy env variable?"; \
		exit 1; \
	fi
	ln -sf $(SRC) src
	patch -p1 -d src < lmbench3.cbench.patch

all: $(SRC)
	$(MAKE) -C src
	#$(MAKE) CB_CC=$(PLAINCC) CB_CFLAGS="$(CFLAGS)" -C src/cachebench cachebench

clean:
	$(MAKE) -C src clean

distclean:
	-/bin/rm -rf $(SRC) $(TAR) src

install uninstall:
	$(do-$@)

