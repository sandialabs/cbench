include ../../make.def

SRC=streams
FILES=stream.c mysecond.c stream.f stream.c.5.10
URL=http://www.cs.virginia.edu/stream/FTP/Code

EXEC = $(SRC)/src/stress
INSTALLDIR = $(hwtest_bin_dir)
INSTALLTARGETS = $(EXEC)

default: .make-$(SRC)

download: $(SRC)

$(SRC):
	@for f in $(FILES); do \
		[ ! -f $$f ] && $(call download-wget-url, $(URL)/$$f); \
	done

mysecond.c stream.c stream.f stream.c.5.10: 
	[ ! -f $$f ] && $(call download-curl-url, $(URL)/$@); \

mysecond.o: mysecond.c
	$(PLAINCC) $(CCFLAGS) -c mysecond.c
	@touch $@

stream_f: stream.f mysecond.o
	$(PLAINF77) $(FFLAGS) -c stream.f
	$(PLAINF77) $(FFLAGS) stream.o mysecond.o -o stream_f

stream_c: stream.c
	$(PLAINCC) $(CCFLAGS) stream.c -o stream_c

stream_c_5_10: stream.c.5.10
	[ -f stream.c.5.10 ] && ln -sf stream.c.5.10 stream_5_10.c
	$(PLAINCC) $(CCFLAGS) stream_5_10.c -o stream_c.5.10

.make-$(SRC): stream_f stream_c 
	touch $@

clean: 
	rm -f mysecond.o  stream_c  stream_f  stream.o .make-$(SRC)

distclean: clean
	rm -f $(FILES) .make-$(SRC) stream_5_10.c


#install uninstall:
#	$(do-$@)



