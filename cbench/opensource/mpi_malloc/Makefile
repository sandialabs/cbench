include ../../make.def

DEFINES = 
LIBS = $(MPILIB)

SRC = mpi_malloc.c
EXEC = mpi_malloc

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXEC)

download:

$(EXEC): $(SRC) meminfo.o
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $< $(LIBS) meminfo.o

clean:
	rm -f *.o $(EXEC)

distclean: clean

install uninstall:
	$(do-$@)
