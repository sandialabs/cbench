include ../../make.def

INSTALLDIR = $(hwtest_bin_dir)
INSTALLTARGETS = stream-* stream2-* stream-mpi

FLAGS = $(CFLAGS)
FFLAGS += $(OPTFLAGS)
MYSECOND=mysecond.o

EXECS = stream-c stream-f stream2-f stream-big-c stream-mpi stream-big-f stream-mpi

ifeq ($(COMPILER),intel)
  EXECS += stream-intel-c stream-intel-f stream-intel2-c stream-intel2-f stream-intel-openmp-f stream-intel-openmp-c stream-intel2-openmp-f stream-intel2-openmp-c stream-mpi-openmp
endif
ifeq ($(COMPILER),gcc)
  EXECS += stream-gcc-c stream-gcc2-c stream-gcc3-c stream-gcc4-c stream-gcc5-c stream-gcc6-c
endif

MPICH_CH = ch_shmem
ifeq ($(CBENCH_STANDALONE),yes)
  $(warning ================ STANDALONE MPI COMPILE)
  EXECS = stream-mpi.$(MPICH_CH)
  MPIHOME = $(CBENCHOME)/opensource/mpich
  INSTALLTARGETS = $(EXECS)
default: stream-mpi.$(MPICH_CH)
endif

all: $(EXECS)

stream-c: stream.c
	$(PLAINCC) $(FLAGS) -o $@ $^ -lm

stream-f: stream.f mysecond.o
	$(FORT) $(FFLAGS) -o $@ $^ -lm

stream2-f: stream2.f mysecond.o
	$(FORT) $(FFLAGS) -o $@ $^ -lm

stream-mpi: stream_mpi.f $(MYSECOND)
	$(F77) $(FFLAGS) -o $@ $^

stream-mpi-openmp: stream_mpi.f $(MYSECOND)
	$(F77) $(FFLAGS) -openmp -o $@ $^

stream-mpi.$(MPICH_CH): stream-mpi
	/bin/mv -f  stream-mpi stream-mpi.$(MPICH_CH)

stream-big-c: stream_big.c
	$(PLAINCC) $(FLAGS) -o $@ $^ -lm

stream-big-f: stream_big.f mysecond.o
	$(F77) $(FFLAGS) -o $@ $^ -lm

# stream binary optimized for GCC
stream-gcc-c: stream_big.c
	$(PLAINCC) $(COMMON_FLAGS) -O3 -mtune=nocona -o $@ $^ -lm
stream-gcc2-c: stream_big.c
	$(PLAINCC) $(COMMON_FLAGS) -O3 -mtune=opteron -o $@ $^ -lm
stream-gcc3-c: stream_big.c
	gcc4 $(COMMON_FLAGS) -O3 -mtune=nocona -o $@ $^ -lm
stream-gcc4-c: stream_big.c
	gcc4 $(COMMON_FLAGS) -O3 -mtune=opteron -o $@ $^ -lm
stream-gcc5-c: stream_big.f mysecond.o
	gfortran $(COMMON_FLAGS) -O3 -mtune=opteron -o $@ $^ -lm
stream-gcc6-c: stream_big.f mysecond.o
	gfortran $(COMMON_FLAGS) -O3 -mtune=nocona -o $@ $^ -lm


# stream binaries optimized for Intel
stream-intel-c: stream_big.c
	$(PLAINCC) $(COMMON_FLAGS) -O3 -axP -ip -fno-alias -o $@ $^ -lm
stream-intel2-c: stream_big.c
	$(PLAINCC) $(COMMON_FLAGS) -O3 -axW -ip -fno-alias -o $@ $^ -lm
stream-intel-f: stream_big.f mysecond.o
	$(PLAINF77) $(COMMON_FLAGS) -O3 -axP -ip -fno-alias -o $@ $^ -lm
stream-intel2-f: stream_big.f mysecond.o
	$(PLAINF77) $(COMMON_FLAGS) -O3 -axW -ip -fno-alias -o $@ $^ -lm
stream-intel-openmp-f: stream.f mysecond.o
	$(PLAINF77) $(COMMON_FLAGS) -openmp -o $@ $^ -lm
stream-intel2-openmp-f: stream_big.f mysecond.o
	$(PLAINF77) $(COMMON_FLAGS) -openmp -o $@ $^ -lm
stream-intel-openmp-c: stream.c
	$(PLAINCC) $(COMMON_FLAGS) -openmp -o $@ $^ -lm
stream-intel2-openmp-c: stream_big.c
	$(PLAINCC) $(COMMON_FLAGS) -openmp -o $@ $^ -lm


mysecond.o: mysecond.c
	$(PLAINCC) $(FLAGS) -o $@ -DUNDERSCORE -c $<


clean:
	rm -f stream-* stream2-* $(MYSECOND) $(MYSECOND_CC) *.o

install uninstall:
	$(do-$@)
