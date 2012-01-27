include ../../make.def

SRC=streams
FILES = stream.c mysecond.c stream.f stream.c.5.10
URL=http://www.cs.virginia.edu/stream/FTP/Code

EXEC = $(SRC)/src/stress
INSTALLDIR = $(hwtest_bin_dir)
INSTALLTARGETS = $(EXEC)

default: .make-$(SRC)

download: $(SRC)

$(SRC):
	@echo "Downloading $(FILES)"
	@for f in $(FILES); do \
		if [ ! -f $$f ]; then \
			echo $(URL)/$$f; \
			$(call download-curl-url, $(URL)/$$f); \
		 fi; \
	done

mysecond.o: $(SRC)
	FILES=mysecond.c
	$(PLAINCC) $(CCFLAGS) -c mysecond.c

stream_f: $(SRC) mysecond.o
	FILES="mysecond.c stream.f"
	$(PLAINF77) $(FFLAGS) -c stream.f
	$(PLAINF77) $(FFLAGS) stream.o mysecond.o -o stream_f

stream_c: $(SRC)
	FILES=stream.c
	$(PLAINCC) $(CCFLAGS) stream.c -o stream_c

stream_c_5_10: $(SRC)
	FILES=stream.c.5.10
	ln -sf stream.c.5.10 stream_5_10.c
	$(PLAINCC) $(CCFLAGS) stream_5_10.c -o stream_c.5.10

.make-$(SRC): stream_f stream_c 
	touch $@

clean: 
	rm -f mysecond.o  stream_c  stream_f  stream.o .make-$(SRC)

distclean: clean
	FILES="stream.c mysecond.c stream.f stream.c.5.10"
	rm -f $(FILES) .make-$(SRC) stream_5_10.c


#install uninstall:
#	$(do-$@)



