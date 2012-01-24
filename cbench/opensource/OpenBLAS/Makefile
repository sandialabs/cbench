include ../../make.def

VERSION=v0.1alpha2.4-0-gfe7a932
SRC=xianyi-OpenBLAS-$(VERSION)
TAR=$(SRC).tar.gz
URL=http://github.com/xianyi/OpenBLAS/tarball/v0.1alpha2.4

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = src/OpenBLAS

default: .make-$(SRC)

$(TAR):
	@echo "Downloading $(TAR)"
	$(call download-url,$(URL))
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
	@if [ $(ARCH) = "x86_64" ]; then cd src && ./quickbuild.64bit; touch $@; \
	elif [ $(ARCH) = "x86" ]; then  cd src && ./quickbuild.32; touch $@; \
	else \
		echo No $(ARCH) found for $(SRC); \
		exit 1;	\
	fi

install: .make-$(SRC)
	mkdir -p $(CBENCHTEST)/$(SRC)
	cp -a src/libgoto* $(CBENCHTEST)/$(SRC) 

distclean: 
	@if [ -f .make-$(SRC) ]; then rm -f .make-$(SRC) ; fi
	@if [ -f ._$(SRC) ]; then rm -f ._$(SRC) ; fi
	@if [ -d $(SRC) ]; then rm -rf $(SRC) ; fi
	@if [ -f $(TAR) ]; then rm -f $(TAR) ; fi
	-/bin/rm -f src

clean:
	$(MAKE) -C src clean

