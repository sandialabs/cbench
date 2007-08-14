include ../../make.def

DEFINES = 
LIBS = $(MPILIB)

SRC = mpi_hello_ordered.c
EXEC = mpi_hello_ordered

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXEC)

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $< $(LIBS)

clean:
	rm -f *.o $(EXEC)

install uninstall:
	$(do-$@)
