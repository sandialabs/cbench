include ../../make.def

SRC=IOR-2.10.3
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
	@echo "Extracting $(SRC)"
	@if [ -f $(TAR) ] ; then tar -zxf $(TAR); \
	else \
		echo "$(TAR) not found, download manually or set http_proxy env variable?"; \
		exit 1; \
	fi
	mv IOR $(SRC)
	ln -s $(SRC)/src/C src

$(TAR):
	@echo "Downloading $(TAR)"
	$(call download-sf-url,$(URL))

clean:
	-[ -d $(SRC)/src/C ] && $(MAKE) -C src CC.Linux=$(CC) $@
	/bin/rm -f IOR.*

distclean: clean
	/bin/rm -rf $(TAR) $(SRC) src

install uninstall:
	$(do-$@)

