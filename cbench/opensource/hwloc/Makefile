include ../../make.def

SRC=hwloc-1.2
TAR=$(SRC).tar.bz2
URL="http://www.open-mpi.org/software/hwloc/v1.2/downloads/$(TAR)"

EXECS = src/utils/lstopo

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

default: all

$(TAR):
	@echo "Downloading $(TAR)"
	$(call download-url,$(URL))

download: $(TAR)

$(SRC): $(TAR)
	@echo "Extracting $(SRC)"
	@if [ -f $(TAR) ] ; then tar -xjf $(TAR); \
	else \
		echo "$(TAR) not found, download manually or set http_proxy env variable?"; \
		exit 1; \
	fi
	ln -sf $(SRC) src

all: $(SRC) src/Makefile
	$(MAKE) -C src

src/Makefile: 
	cd src ; CC=$(CC) CFLAGS='$(CFLAGS)' ./configure

clean:
	$(MAKE) -C src clean

distclean:
	-/bin/rm -rf $(SRC) $(TAR) src

install uninstall:
	$(do-$@)

