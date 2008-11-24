include ../../make.def

SUBDIRS = ./perftest-1.3a

EXECS = $(SUBDIRS)/stress

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

default: $(EXECS)

$(SUBDIRS)/stress:
	cd $(SUBDIRS); export CC=$(CC); export F77=$(F77); export MPILIB=$(MPICH)/lib; ./configure --with-mpich=$(MPICH); make; make stress

clean:
	cd $(SUBDIRS); make clean; make distclean

install uninstall:
	$(do-$@)
