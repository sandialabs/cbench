include ../../make.def

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = SRC_PMB/PMB-MPI1

default all clean:
	cd SRC_PMB; $(MAKE) CC=$(CC) CLINKER='$(CC) $(LDFLAGS)' \
        F90=$(F90) FLINKER='$(F90) $(LDFLAGS)' CPPFLAGS='$(CFLAGS)' \
        LIBS=$(MPILIB) MPI_INCLUDE=. $@

install uninstall:
	$(do-$@)
