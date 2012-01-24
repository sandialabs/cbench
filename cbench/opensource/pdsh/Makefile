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
	@echo "Compiling $(SRC)"
	cd $(SRC) && make -C src CC=$(PLAINCC) > ../.make-$(SRC) 2>&1
	@touch $@

.configure-$(SRC): $(SRC)
	@echo "Configuring $(SRC)"
	cd $(SRC) && ./configure --prefix=$(CBENCHTEST)/$(SRC) --enable-static --enable-static-modules --with-ssh --with-exec --with-readline --without-pam > ../.configure-$(SRC) 2>&1
	@touch $@

clean:
	@if [ -f src/Makefile ] ; then $(MAKE) -C src clean; fi
	rm -f .make-$(SRC) .configure-$(SRC)

distclean:
	rm -fr .make-$(SRC) .configure-$(SRC) src $(TAR) $(SRC)

install: .make-$(SRC)
	@echo "Installing $(SRC)"
	cd src && make install >> ../.make-$(SRC) 2>&1
	@for progs in dshbak  pdcp  pdsh  rpdcp; do \
		ln -sf $(CBENCHTEST)/$(SRC)/bin/$$progs $(CBENCHTEST)/sbin/$$progs; \
	done

