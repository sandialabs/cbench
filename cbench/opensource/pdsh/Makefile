include ../../make.def

VERSION=2.26
SRC=pdsh-$(VERSION)
TAR=$(SRC).tar.bz2
URL="http://sourceforge.net/projects/pdsh/files/pdsh/pdsh-$(VERSION)/$(TAR)"

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = src/pdsh

default: $(SRC)
	cd $(SRC) && ./configure; make -C src CC=$(PLAINCC)

$(TAR):
	@echo "Downloading $(TAR)"
	$(call download-sf-url,$(URL))

download: $(TAR)

$(SRC): $(TAR)
	@echo "Extracting $(SRC)"
	@if [ -f $(TAR) ] ; then tar -jxf $(TAR); \
	else \
		echo "$(TAR) not found, download manually or set http_proxy env variable?"; \
		exit 1; \
	fi
	ln -sf $(SRC) src

clean:
	-[ -d $(SRC) ] && /bin/rm -fr $(SRC)

distclean:
	-/bin/rm -rf $(SRC) $(TAR) src

install uninstall:
	$(do-$@)

