include ../../make.def

SRC=streams
FILES = stream.c mysecond.c stream.f stream.c.5.10
URL="http://www.cs.virginia.edu/stream/FTP/Code"

EXEC = $(SRC)/src/stress
INSTALLDIR = $(hwtest_bin_dir)
INSTALLTARGETS = $(EXEC)

default: stream_f stream_c stream_c_5_10
.make-$(SRC): $(SRC)

download: $(SRC)

$(SRC):
	@echo "Downloading $(FILES)"
	@for f in $(FILES); do \
		if [ ! -f $$f ]; then  $(call download-sf-url, "$(URL)/$$f");  fi \
	done

mysecond.o:
	$(PLAINCC) $(CCFLAGS) -c mysecond.c

stream_f: mysecond.o
	$(PLAINF77) $(FFLAGS) -c stream.f
	$(PLAINF77) $(FFLAGS) stream.o mysecond.o -o stream_f

stream_c: 
	$(PLAINCC) $(CCFLAGS) stream.c -o stream_c

stream_c_5_10: 
	ln -sf stream.c.5.10 stream_5_10.c
	$(PLAINCC) $(CCFLAGS) stream_5_10.c -o stream_c.5.10

clean: 
	rm -f mysecond.o  stream_c  stream.c.5.10  stream_f  stream.o

distclean: clean
	rm -f $(FILES)


#install uninstall:
#	$(do-$@)



