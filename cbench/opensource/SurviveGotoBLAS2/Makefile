include ../../make.def

VERSION=3.14_RC_20110722
SRC=SurviveGotoBLAS2_$(VERSION)
TAR=$(SRC).tar.gz
URL=http://prs.ism.ac.jp/~nakama/SurviveGotoBLAS2/SurviveGotoBLAS2_3.14_RC_20110722.tar.gz

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = src/OpenBLAS

default: .make-$(SRC)

download: $(TAR)

$(TAR):
	$(call download-curl-url,$(URL))

$(SRC): $(TAR)
	$(call untar-src,$(TAR))
	mv survivegotoblas2-3.14 $(SRC)
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
	/bin/rm -fr src .make-$(SRC) ._$(SRC) $(SRC) $(TAR)

clean:
	$(MAKE) -C src clean

