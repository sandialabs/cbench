include ../../make.def

VERSION=v0.1alpha2.4-0-gfe7a932
SRC=xianyi-OpenBLAS-$(VERSION)
TAR=$(SRC).tar.gz
URL=http://github.com/xianyi/OpenBLAS/tarball/v0.1alpha2.4

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = src/OpenBLAS

default: .make-$(SRC)

$(TAR):
	$(call download-curl-url,$(URL))
	mv v0.1alpha2.4 $(TAR)

download: $(TAR)

$(SRC): $(TAR)
	@echo "Extracting $(SRC)"
	@if [ -f $(TAR) ] ; then tar -zxf $(TAR); \
	else \
		echo "$(TAR) not found, download manually or set http_proxy env variable?"; \
		exit 1; \
	fi
	mv xianyi-OpenBLAS-92ba8a7 $(SRC)
	ln -sf $(SRC) src


.make-$(SRC): $(SRC)
	@echo "Compiling $(SRC)"
	@if [ $(ARCH) = "x86_64" ]; then cd src && /bin/sh ./quickbuild.64bit > ../.make-$(SRC) 2>&1; touch $@; \
	elif [ $(ARCH) = "x86" ]; then  cd src && /bin/sh ./quickbuild.32 > ../.make-$(SRC) 2>&1; touch $@; \
	else \
		echo No $(ARCH) found for $(SRC); \
		exit 1;	\
	fi

install: .make-$(SRC)
	@echo "Installing $(SRC)"
	mkdir -p $(CBENCHTEST)/$(SRC)
	cd src && make PREFIX=$(CBENCHTEST)/$(SRC) install >> ../.make-$(SRC) 2>&1
	cd $(CBENCHTEST) && ln -sf $(SRC) OpenBLAS

distclean: 
	rm -fr .make-$(SRC) ._$(SRC) $(SRC) src $(TAR)

clean:
	rm -fr .make-$(SRC) ._$(SRC) $(SRC) src

