include ../../make.def

CFLAGS = -g -DONE_MALLOC -DREALLY_SHORT
LDFLAGS =
FFLAGS = -g

INCLUDES =

SRC = w9.c
EXEC = w9

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = w9 w9ng bug9-new

default: w9 w9ng bug9-new

w9: w9.c
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $< $(LIBS)

w9ng: w9ng.c
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $< $(LIBS)

bug9-new: bug9-new.f90
	$(F90) $(FFLAGS) $(DEFINES) -o $@ $< $(LIBS)

clean:
	rm -f w9 bug9-new w9ng *.o

install uninstall:
	$(do-$@)
