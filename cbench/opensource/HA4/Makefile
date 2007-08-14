include ../../make.def

DEFINES = 
LIBS = $(MPILIB)

SRC = ha4.c
EXEC = ha4

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXEC)

$(EXEC): $(SRC) meminfo.o
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $< $(LIBS) meminfo.o

clean:
	rm -f *.o $(EXEC)

install uninstall:
	$(do-$@)
