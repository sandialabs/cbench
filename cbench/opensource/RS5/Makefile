include ../../make.def

DEFINES = 
LIBS = $(MPILIB)

SRC = rs5.c
EXEC = rs5

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXEC)

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $< $(LIBS)

clean:
	rm -f *.o $(EXEC)

install uninstall:
	$(do-$@)
