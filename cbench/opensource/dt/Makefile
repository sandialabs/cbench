include ../../make.def

VERSION=source
SRC=dt-$(VERSION)
TAR=$(SRC).tar.gz
URL="http://www.scsifaq.org/RMiller_Tools/ftp/dt/$(TAR)"

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = src/dt

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
	mv dt.d-WIP $(SRC)
	ln -sf $(SRC) src

all: $(SRC)
	cp -f src/Makefile.linux src/Makefile
	$(MAKE) -C src CC=$(PLAINCC) 

clean:
	$(MAKE) -C src clean

distclean:
	-/bin/rm -rf $(SRC) $(TAR) src

install uninstall:
	$(do-$@)

