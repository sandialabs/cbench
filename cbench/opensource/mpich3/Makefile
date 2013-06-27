include ../../make.def

VERSION = 3.0.4
SRC=mpich-$(VERSION)
TAR=$(SRC).tar.gz
URL=http://www.mpich.org/static/downloads/$(VERSION)/$(TAR)

default: .make-$(SRC)

download: $(TAR)

$(TAR):
	$(call download-curl-url,$(URL))

$(SRC): $(TAR)
	$(call untar-src,$(TAR))
	ln -sf $@ src
	@[ -d $@ ] && touch $@

# from ompi 1.4.X&& export CC=$(PLAINCC) && F77=$(PLAINF77) && FC=$(PLAINF77) && CXX=$(PLAINCXX) && \ 
#	 ./configure --enable-orterun-prefix-by-default --enable-mpi-threads --prefix=$(BENCH_TEST)/openmpi > ../.configure-$(SRC) 2>&1)
.configure-$(SRC): $(SRC)
	@echo "Configuring $(SRC) ... NOT" && exit 1
	(cd src && export CC=$(PLAINCC) && export F77=$(PLAINF77) && export FC=$(PLAINF77) && export CXX=$(PLAINCXX) && ./configure --enable-orterun-prefix-by-default --with-threads=posix --enable-mpi-thread-multiple --enable-opal-multi-threads --prefix=$(CBENCHTEST)/openmpi > ../.configure-$(SRC) 2>&1)
	@touch $@

.make-$(SRC): .configure-$(SRC)
	@echo "Compiling $(SRC)"
	$(MAKE) -C src -j 3 > .make-$(SRC) 2>&1; touch .configure-$(SRC)
	@touch $@ 

clean: .configure-$(SRC)
	$(MAKE) -C src clean > .make-$(SRC) 2>&1
	rm -f .make-$(SRC) 

distclean:
	rm -fr .make-$(SRC) .configure-$(SRC) src $(TAR) $(SRC) .install-$(SRC)

.install-$(SRC): .make-$(SRC)
	@echo "Installing $(SRC)"
	$(MAKE) -C src PREFIX=$(CBENCHTEST)/openmpi install >> .make-$(SRC) 2>&1
	@touch $@ 

install: .install-$(SRC)

reinstall: clean_install install

clean_install:
	rm -f .install-$(SRC)
