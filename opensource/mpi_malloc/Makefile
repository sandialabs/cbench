include ../../make.def

DEFINES = 
LIBS = $(MPILIB)

SRC = mpi_malloc.c
EXEC = mpi_malloc

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXEC)

$(EXEC): $(SRC) meminfo.o
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $< $(LIBS) meminfo.o

clean:
	rm -f *.o $(EXEC)

install uninstall:
	$(do-$@)
