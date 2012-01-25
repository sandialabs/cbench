include ../../make.def

VERSION=9.37
SRC=hdparm-$(VERSION)
TAR=$(SRC).tar.gz
URL="http://sourceforge.net/projects/hdparm/files/hdparm/$(TAR)"

BUILDLOG=cbench_buildlog_$(SRC).log

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = src/hdparm

default: $(SRC)
	make -C src CC=$(PLAINCC) > $(BUILDLOG) 2>&1

download: $(TAR)

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
	cd src && make clean 

distclean:
	-/bin/rm -rf $(SRC) $(TAR) src $(BUILDLOG)

install uninstall:
	$(do-$@)

