include ../../make.def

VERSION=9.37
SRC=hdparm-$(VERSION)
TAR=$(SRC).tar.gz
URL="http://sourceforge.net/projects/hdparm/files/hdparm/$(TAR)"

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = src/hdparm

download: $(TAR)

default: $(SRC)
	make -C src CC=$(PLAINCC) 

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
	-[ -d $(SRC) ] && /bin/rm -fr $(SRC)

distclean:
	-/bin/rm -rf $(SRC) $(TAR) src

install uninstall:
	$(do-$@)

