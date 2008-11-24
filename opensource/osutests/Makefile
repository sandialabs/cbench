include ../../make.def

DEFINES = 
CFLAGS += 
LIBS = $(MPILIB) -lm

SRC = mpi_tokensmash.c
EXECS = osu_bw osu_bibw osu_latency osu_bcast osu_mbw_mr

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

default: $(EXECS)

osu_bw: osu_bw.c 
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $< $(LIBS)

osu_bibw: osu_bibw.c 
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $< $(LIBS)

osu_mbw_mr: osu_mbw_mr.c
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $< $(LIBS)

osu_latency: osu_latency.c 
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $< $(LIBS)

osu_bcast: osu_bcast.c 
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $< $(LIBS)

clean:
	rm -f *.o $(EXECS)

install uninstall:
	$(do-$@)
