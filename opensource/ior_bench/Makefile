include ../../make.def

SRC=IOR-2.10.2
TAR=$(SRC).tgz
URL="http://downloads.sourceforge.net/project/ior-sio/IOR%20latest/$(SRC)/$(SRC).tgz"


INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(shell ls IOR.*)

all: $(SRC) IOR.posix IOR.mpiio


IOR.posix:
	$(MAKE) -C src CC.Linux=$(CC) posix
	/bin/cp -f src/IOR $@

IOR.mpiio:
	$(MAKE) -C src CC.Linux=$(CC) mpiio
	/bin/cp -f src/IOR $@

$(SRC): $(TAR)
	[ ! -d $(SRC) ] && tar zxf $(TAR)
	touch $(SRC)
	ln -s $(SRC)/src/C src

$(TAR) download:
	$(call download-url,$(URL))
	-[ -e $(TAR) ] && touch $(TAR)

clean:
	-[ -d $(SRC)/src/C ] && $(MAKE) -C src CC.Linux=$(CC) $@
	/bin/rm -f IOR.*

distclean: clean
	/bin/rm -rf $(TAR) $(SRC) src

install uninstall:
	$(do-$@)
