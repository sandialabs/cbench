include ../../make.def

VERSION=1.4
SRC=conceptual-$(VERSION)
TAR=$(SRC).tar.gz
URL="http://sourceforge.net/projects/conceptual/files/conceptual/$(VERSION)/$(TAR)"

default: .make-$(SRC)

download: $(TAR)

$(TAR):
	$(call download-curl-url,$(URL))

$(SRC): $(TAR)
	@echo "Extracting $(SRC)"
	@if [ -f $(TAR) ] ; then tar -zxf $(TAR); \
	else \
		echo "$(TAR) not found, download manually or set http_proxy env variable?"; \
		exit 1; \
	fi
	ln -sf $(SRC) src

.configure-$(SRC): $(SRC)
	@echo "Configuring $(SRC)"
	cd src && MPICC=$(MPIHOME)/bin/mpicc ./configure --prefix=$(CBENCHTEST)/$(SRC) > ../.configure-$(SRC) 2>&1
	@touch $@

.make-$(SRC): .configure-$(SRC)
	@echo "Compiling $(SRC)"
	cd src && make > ../.make-$(SRC) 2>&1
	@touch $@

clean:
	@if [ -f src/Makefile ] ; then $(MAKE) -C src clean; fi
	rm -f .make-$(SRC) .configure-$(SRC)

distclean:
	rm -fr .make-$(SRC) .configure-$(SRC) src $(TAR) $(SRC)

install: .make-$(SRC)
	@echo "Installing $(SRC)"
	cd src && make install >> ../.make-$(SRC) 2>&1
