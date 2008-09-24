include $(CBENCHOME)/make.def

SRC=sweep3d-2.2b
TAR=sweep3d-2.2b.tar.gz
URL=http://www.ccs3.lanl.gov/pal/software/sweep3d/$(TAR)


EXECS = $(SRC)/sweep
INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)
FFLAGS = -fno-second-underscore -O3

all: $(SRC) src/Makefile
	make -C $(SRC)

src/Makefile:
	cd src && ./configure --with-mpi F77=$(F77) CC=$(CC) FFLAGS="$(FFLAGS)" CFLAGS="$(CFLAGS)"

$(SRC) src: $(TAR)
	[ ! -d $(SRC) ] && tar zxf $(TAR)
	ln -sf $(SRC) src
	touch $(SRC)

$(TAR) download:
	$(call download-url,$(URL))
	-[ -e $(TAR) ] && touch $(TAR)

clean:
	make -C $(SRC) clean

distclean:
	/bin/rm -rf $(TAR) $(SRC) src

install uninstall:
	$(do-$@)
