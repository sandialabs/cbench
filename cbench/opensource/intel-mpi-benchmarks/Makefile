include ../../make.def

VERSION=3.2.3
SRC=IMB_$(VERSION)
TAR=$(SRC).tgz
URL="http://software.intel.com/sites/products/mpi-benchmarks/$(TAR)"

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = 

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
	cd src && make -C src CC=$(CC) > ../.make-$(SRC) 2>&1
	@touch $@

.configure-$(SRC): $(SRC)
	@echo "Configuring $(SRC)"
	cd src && ./configure --prefix=$(CBENCHTEST)$(SRC) > ../.configure-$(SRC) 2>&1
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

