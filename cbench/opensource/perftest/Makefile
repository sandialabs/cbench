include ../../make.def

VERSION=1.5
SRC=perftest-$(VERSION)
TAR=$(SRC).tar.gz
URL="http://ftp.mcs.anl.gov/pub/mpi/tools/$(TAR)"

EXECS = src/stress
INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

# since the perftest configure snapshots the Cbench COMPILERCOLLECTION
# and MPIHOME state, if the COMPILERCOLLECTION changes and perftest has
# already been configured, we need to force a reconfigure
#ifeq ($(wildcard .config), .config)
#ifneq ($(shell cat .config), ${COMPILERCOLLECTION})
#	@echo "COMPILERCOLLECTION changed, forcing reconfigure.."
#	@/bin/rm -f .config src/Makefile $(EXECS)
#	$(MAKE)
#endif
#endif

default: $(EXECS)

$(TAR):
	@echo "Downloading $(TAR)"
	$(call download-url,$(URL))

download: $(TAR)

$(SRC): $(TAR)
	@echo "Extracting $(SRC)"
	@if [ -f $(TAR) ] ; then tar -zxf $(TAR); \
	else \
		echo "$(TAR) not found, download manually or set http_proxy env variable?"; \
		exit 1; \
	fi
	ln -sf $(SRC) src

configure: $(SRC)
	echo ${COMPILERCOLLECTION} > src/.config
	cd src; CC=$(PLAINCC) F77=$(PLAINF77) CFLAGS='$(CFLAGS)' FFLAGS='$(FFLAGS)' ./configure --with-mpi=$(MPIHOME)

src/stress: configure
	make -C src stress

clean:
	$(MAKE) -C src clean

distclean:
	-/bin/rm -rf $(SRC) $(TAR) src .config

install uninstall:
	$(do-$@)
