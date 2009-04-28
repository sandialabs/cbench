include ../../make.def

SUBDIR = ./perftest-1.3a

EXECS = $(SUBDIR)/stress

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

default: $(EXECS)

# since the perftest configure snapshots the Cbench COMPILERCOLLECTION
# and MPIHOME state, if the COMPILERCOLLECTION changes and perftest has
# already been configured, we need to force a reconfigure
ifeq ($(wildcard .config), .config)
ifneq ($(shell cat .config), ${COMPILERCOLLECTION})
	@echo "COMPILERCOLLECTION changed, forcing reconfigure.."
	@/bin/rm -f .config $(SUBDIR)/Makefile $(EXECS)
	$(MAKE)
endif
endif

$(SUBDIR)/stress: $(SUBDIR)/Makefile $(SUBDIR)/stress.c
	make -C $(SUBDIR) stress

$(SUBDIR)/Makefile:
	echo ${COMPILERCOLLECTION} > .config
	cd $(SUBDIR); export CC=$(CC); export F77=$(F77); \
		export CFLAGS='$(CFLAGS)'; export FFLAGS='$(FFLAGS)'; \
		./configure --with-mpich=

clean:
	make -C $(SUBDIR) clean

distclean: clean
	make -C $(SUBDIR) distclean
	/bin/rm -f .config

install uninstall:
	$(do-$@)
