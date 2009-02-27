include ../../make.def

SUBDIRS = ./bonnie++-1.03d

EXECS = $(SUBDIRS)/bonnie++

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

default: $(EXECS)

$(SUBDIRS)/bonnie++: untar
	cd $(SUBDIRS); export CC=$(CC); export F77=$(F77); export MPILIB=$(MPICH)/lib; if [ ! -f config.status ]; then ./configure; fi; make

untar:
	if [ ! -d bonnie++-1.03d ]; then tar -zxvf bonnie++-1.03d.tgz; fi
clean:
	cd $(SUBDIRS); make realclean

distclean: clean

install uninstall:
	$(do-$@)
