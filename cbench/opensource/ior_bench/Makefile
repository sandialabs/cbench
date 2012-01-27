include ../../make.def

SRC=IOR-2.10.3
TAR=$(SRC).tgz
URL="http://downloads.sourceforge.net/project/ior-sio/IOR%20latest/$(SRC)/$(SRC).tgz"


INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(shell ls IOR.*)

all: $(SRC) IOR.posix IOR.mpiio

download: $(TAR)

$(TAR):
	$(call download-curl-url,$(URL))

$(SRC): $(TAR)
	$(call untar-src,$(TAR))
	mv IOR $(SRC)
	ln -s $(SRC)/src/C src

IOR.posix:
	$(MAKE) -C src CC.Linux=$(CC) posix
	/bin/cp -f src/IOR $@

IOR.mpiio:
	$(MAKE) -C src CC.Linux=$(CC) mpiio
	/bin/cp -f src/IOR $@

clean:
	-[ -d $(SRC)/src/C ] && $(MAKE) -C src CC.Linux=$(CC) $@
	/bin/rm -f IOR.*

distclean: clean
	/bin/rm -rf $(TAR) $(SRC) src

install uninstall:
	$(do-$@)
