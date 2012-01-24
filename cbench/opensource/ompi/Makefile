include ../../make.def

VERSION = 1.4.4
SRC=openmpi-1.4.4
TAR=$(SRC).tar.bz2
URL=http://www.open-mpi.org/software/ompi/v1.4/downloads/$(TAR)

default: .make-$(SRC)

$(TAR):
	@echo "Downloading $(SRC)"
	$(call download-url,$(URL))
	-[ -e $(TAR) ] && touch $(TAR)

download: $(TAR)

.make-$(SRC): .configure-$(SRC)
	@echo "Compiling $(SRC)"
	cd $(SRC) && make -j 3 > ../.make-$(SRC) 2>&1
	@touch $@

.configure-$(SRC): $(SRC)
	@echo "Configuring $(SRC)"
	(cd $(SRC); export CC=$(PLAINCC); export F77=$(PLAINF77); \
	 export FC=$(PLAINF77); export CXX=$(PLAINCXX); \
	 ./configure --enable-orterun-prefix-by-default --enable-mpi-threads --prefix=$(BENCH_TEST)/openmpi > ../.configure-$(SRC) 2>&1)
	@touch $@

# target to untar the openmpi distribution
$(SRC): $(TAR)
	@echo "Extracting $(SRC)"
	@if [ -f $(TAR) ] ; then tar -jxf $(TAR); \
	else \
		echo "$(TAR) not found, download manually or set http_proxy env variable?"; \
		exit 1; \
	fi

install: .make-$(SRC)
	@echo "Installing $(SRC)"
	cd $(SRC) && make PREFIX=$(CBENCHTEST)/openmpi install >> ../.make-$(SRC) 2>&1

clean:
	@if [ -f $(SRC)/Makefile ] ; then $(MAKE) -C $(SRC) clean; fi
	rm -f .make-$(SRC) .configure-$(SRC)

distclean: 
	rm -f .make-$(SRC) .configure-$(SRC)
	rm -rf $(SRC) $(TAR)
