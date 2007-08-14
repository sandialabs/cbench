include ../../make.def

DEFINES = 
LIBS = $(MPILIB)

SRC = mpi_300mb.c
EXEC = mpi_300mb

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXEC)

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $< $(LIBS)

clean:
	rm -f *.o $(EXEC)

install uninstall:
	$(do-$@)
