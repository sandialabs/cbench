include ../../make.def

SUBDIR = ./perftest-1.3a

EXECS = $(SUBDIR)/stress

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

default: $(EXECS)

$(SUBDIR)/stress: $(SUBDIR)/Makefile $(SUBDIR)/stress.c
	make -C $(SUBDIR) stress

$(SUBDIR)/Makefile:
	cd $(SUBDIR); export CC=$(CC); export F77=$(F77); export CFLAGS='$(CFLAGS)'; ./configure --with-mpich=

clean:
	make -C $(SUBDIR) clean

distclean: clean
	make -C $(SUBDIR) distclean

install uninstall:
	$(do-$@)
