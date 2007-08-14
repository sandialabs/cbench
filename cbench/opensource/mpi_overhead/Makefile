include ../../make.def

DEFINES = 
LIBS = $(MPILIB)

EXEC = mpi_overhead mpi_overhead_bcast

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXEC)

default: $(EXEC)

mpi_overhead: mpi_overhead.o meminfo.o
	$(CC) $(COMMON_FLAGS) $(DEFINES) -o $@ $< $(LIBS) meminfo.o

mpi_overhead_bcast: mpi_overhead_bcast.o meminfo.o
	$(CC) $(COMMON_FLAGS) $(DEFINES) -o $@ $< $(LIBS) meminfo.o

clean:
	rm -f *.o $(EXEC)

install uninstall:
	$(do-$@)
