include ../../make.def

DEFINES = 
#DEFINES += -DNOalltoallv
LIBS = $(MPILIB) -lm

SRC = mpi_hello.c
EXEC = mpi_hello

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXEC)

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $< $(LIBS)

clean:
	rm -f *.o $(EXEC)

install uninstall:
	$(do-$@)
