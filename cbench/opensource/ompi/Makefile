include ../../make.def

VERSION = 1.4.4
OPENMPI = openmpi-$(VERSION)

SRC=openmpi-1.4.4
TAR=$(SRC).tar.bz2
URL=http://www.open-mpi.org/software/ompi/v1.4/downloads/$(TAR)

default: .make-$(OPENMPI)

$(TAR):
	@echo "Downloading $(OPENMPI)"
	$(call download-url,$(URL))
	-[ -e $(TAR) ] && touch $(TAR)

download: $(TAR)

.make-$(OPENMPI): .configure-$(OPENMPI)
	@echo "make $(OPENMPI)"
	(cd $(OPENMPI); make -j 3 && make install)
	touch $@

.configure-$(OPENMPI): $(OPENMPI)
	@echo "Configuring $(OPENMPI)"
	(cd $(OPENMPI); export CC=$(PLAINCC); export F77=$(PLAINF77); \
	 export FC=$(PLAINF77); export CXX=$(PLAINCXX); \
	 ./configure --enable-orterun-prefix-by-default --enable-mpi-threads --prefix=$(BENCH_TEST)/openmpi)
	@touch $@

# target to untar the openmpi distribution
$(OPENMPI): $(TAR)
	@echo "Extracting $(OPENMPI)"
	@if [ -f $(TAR) ] ; then tar -jxf $(TAR); \
	else \
		echo "$(TAR) not found, download manually or set http_proxy env variable?"; \
		exit 1; \
	fi

install: .make-$(OPENMPI)
	cd $(OPENMPI) && make PREFIX=$(CBENCHTEST)/openmpi install

clean:
	@if [ -f $(OPENMPI)/Makefile ] ; then $(MAKE) -C $(OPENMPI) clean; fi
	rm -f .make-$(OPENMPI) .configure-$(OPENMPI)

distclean: 
	rm -f .make-$(OPENMPI) .configure-$(OPENMPI)
	rm -rf $(SRC) $(TAR)
