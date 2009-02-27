include ../../make.def

SRC = nodeperf.c nodeperf2.c xerbla.c dclock.c
OBJS = xerbla.o dclock.o

OPFLAGS += -g

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = nodeperf2-nompi nodeperf nodeperf2 

default: $(INSTALLTARGETS)

# standard Intel nodeperf
nodeperf: $(SRC) $(OBJS)
	$(CC) $(OPTFLAGS) -o $@ nodeperf.c $(OBJS) $(BLASLIB)

# modified nodeperf with MPI
nodeperf2: $(SRC) $(OBJS)
	$(CC) $(OPTFLAGS) -o $@ nodeperf2.c $(OBJS) $(BLASLIB)

# modified nodeperf without MPI 
nodeperf2-nompi: $(SRC) $(OBJS)
	$(PLAINCC) $(OPTFLAGS) -DNOMPI -o $@ nodeperf2.c $(OBJS) $(BLASLIB)

xerbla.o:
	$(PLAINCC) $(OPTFLAGS) -c xerbla.c

dclock.o: dclock.c
	$(PLAINCC) $(OPTFLAGS) -D_LINUX -D_GCC_ -DGETCPUFREQUENCY=get_frequency -DDSECND=dsecnd_ -c dclock.c

%.o: %.c
	$(CC) $(OPTFLAGS) -c $<

nodeperf.c:
ifeq "$(wildcard $(BENCH_HOME)/restricted/nodeperf/nodeperf.c)" "$(BENCH_HOME)/restricted/nodeperf/nodeperf.c"
	@echo "NOTE: Found Cbench restricted tree, grabbing nodeperf.c..."
	@cp $(BENCH_HOME)/restricted/nodeperf/nodeperf.c .
else
  ifeq "$(wildcard nodeperf.c)" ""
	  @echo
	  @echo "ERROR:  Please download Intel's MP Linpack package from the following address,"
	  @echo "ERROR:  extract nodeperf.c to this directory, and try the make again:"
	  @echo "ERROR:"
	  @echo "ERROR:  http://www.intel.com/software/products/mkl/linpack.htm"
	  @echo
	  @false
  else
  endif
endif

nodeperf2.c:
ifeq "$(wildcard $(BENCH_HOME)/restricted/nodeperf/nodeperf2.c)" "$(BENCH_HOME)/restricted/nodeperf/nodeperf2.c"
	@echo "NOTE: Found Cbench restricted tree, grabbing nodeperf2.c..."
	@cp $(BENCH_HOME)/restricted/nodeperf/nodeperf2.c .
else
  ifeq "$(wildcard nodeperf2.c)" ""
	  @echo
	  @echo "ERROR:  You'll need the Cbench Restricted tree to get nodeperf2..."
	  @echo
	  @false
  else
  endif
endif

clean:
	rm -f $(INSTALLTARGETS) nodeperf.o nodeperf2.o $(OBJS)

realclean: clean
	rm -f nodeperf.c nodeperf2.c

distclean: clean

install uninstall:
	$(do-$@)
