include ../../make.def

VERSION=2.26
SRC=pdsh-$(VERSION)
TAR=$(SRC).tar.bz2
URL="http://sourceforge.net/projects/pdsh/files/pdsh/pdsh-$(VERSION)/$(TAR)"

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = src/src/pdsh/pdsh

default: .make-$(SRC)

download: $(TAR)

$(TAR):
	@echo "Downloading $(TAR)"
	$(call download-sf-url,$(URL))

$(SRC): $(TAR)
	@echo "Extracting $(SRC)"
	@if [ -f $(TAR) ] ; then tar -jxf $(TAR); \
	else \
		echo "$(TAR) not found, download manually or set http_proxy env variable?"; \
		exit 1; \
	fi
	ln -sf $(SRC) src

.make-$(SRC): .configure-$(SRC)
	@echo "make $(SRC)"
	cd $(SRC) && make -C src CC=$(PLAINCC)
	@touch $@

.configure-$(SRC): $(SRC)
	@echo "Configuring $(SRC)"
	cd $(SRC) && ./configure --enable-static --enable-static-modules --with-ssh --with-exec --with-readline --without-pam; make -C src CC=$(PLAINCC)
	@touch $@

clean:
	@if [ -f src/Makefile ] ; then $(MAKE) -C src clean; fi
	rm -f .make-$(SRC) .configure-$(SRC)

distclean:
	rm -fr .make-$(SRC) .configure-$(SRC) src $(TAR) $(SRC)

install uninstall: .make-$(SRC)
	$(do-$@)
