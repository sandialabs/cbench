include ../../make.def

DEFINES = 
LIBS = $(MPILIB)

SRC = mp3.c
EXEC = mp3

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXEC)

$(EXEC): second_wall.o $(SRC)
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $^ $(LIBS)

second_wall.o: second_wall.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f *.o $(EXEC)

install uninstall:
	$(do-$@)
