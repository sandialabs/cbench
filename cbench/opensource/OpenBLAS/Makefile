include ../../make.def

VERSION=v0.1alpha2.4-0-gfe7a932
SRC=xianyi-OpenBLAS-$(VERSION)
TAR=$(SRC).tar.gz
URL=http://github.com/xianyi/OpenBLAS/tarball/v0.1alpha2.4

LAPACKSRC=lapack-3.1.1.tgz
LAPACKURL=http://www.netlib.org/lapack/$(LAPACKSRC)
LARGESRC=large.tgz
LARGEURL=http://www.netlib.org/lapack/timing/$(LARGESRC)
TIMINGSRC=timing.tgz
TIMINGURL=http://www.netlib.org/lapack/timing/$(TIMINGSRC)

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = src/OpenBLAS

default: .make-$(SRC)

download: $(TAR)

$(TAR):
	$(call download-curl-url,$(LAPACKURL))
	$(call download-curl-url,$(LARGEURL))
	$(call download-curl-url,$(TIMINGURL))
	$(call download-curl-url,$(URL))
	mv v0.1alpha2.4 $(TAR)

$(SRC): $(TAR)
	$(call untar-src,$(TAR))
	mv xianyi-OpenBLAS-92ba8a7 $(SRC)
	ln -sf $@ src
	mv $(LAPACKSRC) src/
	mv $(LARGESRC) src/
	mv $(TIMINGSRC) src/
	@[ -d $@ ] && touch $@


.make-$(SRC): $(SRC)
	@echo "Compiling $(SRC)"
	@if [ $(ARCH) = "x86_64" ]; then cd src && /bin/sh ./quickbuild.64bit > ../.make-$(SRC) 2>&1; touch $@; \
	elif [ $(ARCH) = "x86" ]; then  cd src && /bin/sh ./quickbuild.32 > ../.make-$(SRC) 2>&1; touch $@; \
	else \
		echo No $(ARCH) found for $(SRC); \
		exit 1;	\
	fi

distclean: 
	rm -fr .make-$(SRC) ._$(SRC) $(SRC) src $(TAR) .install-$(SRC)

clean:
	rm -fr .make-$(SRC) ._$(SRC) $(SRC) src

.install-$(SRC): .make-$(SRC)
	@echo "Installing $(SRC)"
	mkdir -p $(CBENCHTEST)/$(SRC)
	cd src && make PREFIX=$(CBENCHTEST)/$(SRC) install >> ../.make-$(SRC) 2>&1
	cd $(CBENCHTEST) && ln -sf $(SRC) OpenBLAS
	@touch $@

install: .install-$(SRC)

reinstall: clean_install install

clean_install:
	rm -f .install-$(SRC)
