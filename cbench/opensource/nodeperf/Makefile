include ../../make.def

SRC = nodeperf.c xerbla.c dclock.c
INTERNALSRC = nodeperf.c nodeperf2.c xerbla.c dclock.c
OBJS = xerbla.o dclock.o

OPFLAGS += -g

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = nodeperf 
INTERNALINSTALLTARGETS = nodeperf2-nompi nodeperf  nodeperf2

NODEPERFILE=$(CBENCHOME)/opensource/HPLintel/src/../mp_linpack/nodeperf.c

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
	@if [ ! -f $(NODEPERFILE) ]; then \
		make -C $(CBENCHOME)/opensource/HPLintel extractnodeperf; \
	fi
	cp $(NODEPERFILE) $(CBENCHOME)/opensource/nodeperf

nodeperf2.c:
	$(call download-url,http://synapse.sandia.gov/static/tarballs/nodeperf2.c)
	@if [ -e nodeperf.c ]; then \
		touch nodeperf.c;\
	else \
		echo; \
		echo "ERROR:  You'll need the Cbench Restricted tree to get nodeperf2.c ..."; \
		false; \
	fi

download: nodeperf.c nodeperf2.c

clean:
	rm -f $(INSTALLTARGETS) nodeperf.o nodeperf2.o $(OBJS)

distclean: clean
	rm -f nodeperf.c nodeperf2.c

install uninstall:
	$(do-$@)
