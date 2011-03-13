include ../../make.def

VERSION=1.8.3
SRC=mdtest-$(VERSION)
TAR=$(SRC).tgz
URL="http://sourceforge.net/projects/mdtest/files/mdtest%20latest/$(SRC)/$(TAR)"

EXECS = $(SRC)/mdtest
INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

all normal: $(EXECS)

$(EXECS): $(SRC)
	$(MAKE) CC=$(CC) -C $(SRC)

$(TAR):
	@echo "Downloading $(TAR)"
	$(call download-sf-url,$(URL))

$(SRC): $(TAR)
	@echo "Extracting $(SRC)"
	@if [ -f $(TAR) ] ; then tar -zxf $(TAR); \
	else \
		echo "$(TAR) not found, download manually or set http_proxy env variable?"; \
		exit 1; \
	fi
	ln -sf $(SRC) src

clean:
	$(MAKE) -C src clean

distclean:
	/bin/rm -rf $(SRC) $(TAR) src

install uninstall:
	$(do-$@)

