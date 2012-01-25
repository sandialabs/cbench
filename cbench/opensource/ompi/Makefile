include ../../make.def

VERSION = 1.5.4
SRC=openmpi-$(VERSION)
TAR=$(SRC).tar.bz2
URL=http://www.open-mpi.org/software/ompi/v1.5/downloads/$(TAR)

default: .make-$(SRC)

download: $(TAR)

$(TAR):
	@echo "Downloading $(SRC)"
	$(call download-sf-url,$(URL))

$(SRC): $(TAR)
	@echo "Extracting $(SRC)"
	@if [ -f $(TAR) ] ; then tar -jxf $(TAR); \
	else \
		echo "$(TAR) not found, download manually or set http_proxy env variable?"; \
		exit 1; \
	fi
	ln -sf $(SRC) src

# from ompi 1.4.X&& export CC=$(PLAINCC) && F77=$(PLAINF77) && FC=$(PLAINF77) && CXX=$(PLAINCXX) && \ 
#	 ./configure --enable-orterun-prefix-by-default --enable-mpi-threads --prefix=$(BENCH_TEST)/openmpi > ../.configure-$(SRC) 2>&1)
.configure-$(SRC): $(SRC)
	@echo "Configuring $(SRC)"
	(cd src && export CC=$(PLAINCC) && export F77=$(PLAINF77) && export FC=$(PLAINF77) && export CXX=$(PLAINCXX) && ./configure --enable-orterun-prefix-by-default --with-threads=posix --enable-mpi-thread-multiple --enable-opal-multi-threads --prefix=$(CBENCHTEST)/openmpi > ../.configure-$(SRC) 2>&1)
	@touch $@


.make-$(SRC): .configure-$(SRC)
	@echo "Compiling $(SRC)"
	cd src && make -j 3 > ../.make-$(SRC) 2>&1
	@touch $@

install: .make-$(SRC)
	@echo "Installing $(SRC)"
	cd src && make PREFIX=$(CBENCHTEST)/openmpi install >> ../.make-$(SRC) 2>&1

clean:
	@if [ -f src/Makefile ] ; then $(MAKE) -C src clean; fi
	rm -f .make-$(SRC) .configure-$(SRC)

distclean:
	rm -fr .make-$(SRC) .configure-$(SRC) src $(TAR) $(SRC)

