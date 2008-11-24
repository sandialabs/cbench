include ../../make.def

SUBDIRS = src/C

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(shell ls IOR.*)

default all: IOR.posix IOR.mpiio
lustre: IOR.posix-lustre IOR.mpiio-lustre

IOR.posix:
	cd src/C; $(MAKE) CC.Linux=$(CC) LDFLAGS.Linux=-L$(MPICH)/lib posix
	/bin/cp -f src/C/IOR $@

IOR.posix-lustre:
	cd src/C; $(MAKE) CC.Linux=$(CC) LDFLAGS.Linux=-L$(MPICH)/lib clean
	cd src/C; $(MAKE) CC.Linux=$(CC) LDFLAGS.Linux=-L$(MPICH)/lib CCFLAGS.Linux='-g -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_USE_LUSTRE' posix 
	/bin/cp -f src/C/IOR $@

IOR.mpiio:
	cd src/C; $(MAKE) CC.Linux=$(CC) LDFLAGS.Linux=-L$(MPICH)/lib mpiio
	/bin/cp -f src/C/IOR $@

IOR.mpiio-lustre:
	cd src/C; $(MAKE) CC.Linux=$(CC) LDFLAGS.Linux=-L$(MPICH)/lib clean
	cd src/C; $(MAKE) CC.Linux=$(CC) LDFLAGS.Linux=-L$(MPICH)/lib CCFLAGS.Linux='-g -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_USE_LUSTRE' mpiio 
	/bin/cp -f src/C/IOR $@

clean:
	cd src/C; $(MAKE) CC.Linux=$(CC) LDFLAGS.Linux=-L$(MPICH)/lib $@
	/bin/rm -f IOR.*

install uninstall:
	$(do-$@)
