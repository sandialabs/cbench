include $(CBENCHOME)/make.def


CCFLAGS = -DFFTW -I$(FFTWINCLUDE) $(CFLAGS)
LINKFLAGS =	$(FFTWLIB)

dummy_fftw: dummy.c
	$(CC) -DFFTW -I$(FFTWINCLUDE) $(CFLAGS) -c -o $@.o $<
	$(CC) dummy_fftw.o -lfftw $(FFTWLIB) 
