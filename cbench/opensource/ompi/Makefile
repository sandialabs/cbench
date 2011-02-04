include ../../make.def

VERSION = 1.4.3
OPENMPI = openmpi-$(VERSION)

SRC=openmpi-1.4.3
TAR=$(SRC).tar.bz2
URL=http://www.open-mpi.org/software/ompi/v1.4/downloads/$(TAR)

default: .make-$(OPENMPI)

$(TAR) download:
	$(call download-url,$(URL))
	-[ -e $(TAR) ] && touch $(TAR)

.make-$(OPENMPI): .configure-$(OPENMPI)
	(cd $(OPENMPI); make && make install)
	touch $@

.configure-$(OPENMPI): $(OPENMPI)
	(cd $(OPENMPI); export CC=$(PLAINCC); export F77=$(PLAINF77); \
	 export FC=$(PLAINF77); export CXX=$(PLAINCXX); \
	 export CFLAGS='-O1'; export LDFLAGS='-lpthread'; \
	 ./configure --enable-orterun-prefix-by-default \
	 --prefix=$(BENCH_TEST)/openmpi)
	@touch $@

# target to untar the openmpi distribution
$(OPENMPI): $(TAR)
	@echo getting source for $(OPENMPI) ...
	@if [ -f $(OPENMPI).tar.gz ] ; then				\
		zcat $(OPENMPI).tar.gz | tar xf -;			\
	elif [ -f $(OPENMPI).tar.bz2 ] ; then 			\
		bzcat $(OPENMPI).tar.bz2 | tar xf -;			\
	elif [ -f $(OPENMPI).tar ] ; then 				\
		cat $(OPENMPI).tar | tar xf -;			\
	else								\
		echo No $(OPENMPI) found;				\
		exit 1;							\
	fi
#	for p in `ls ../patch* | grep $(VERSION)` ; do	\
#		patch -N -p1 < $$p ;	\
#	done;

install:
	cd $(OPENMPI) && make PREFIX=$(CBENCHTEST)/openmpi install
#	rm -rf $(CBENCHTEST)/mpich/doc $(CBENCHTEST)/mpich/share $(CBENCHTEST)/mpich/etc $(CBENCHTEST)/mpich/examples $(CBENCHTEST)/mpich/man $(CBENCHTEST)/mpich/www

clean:
	@if [ -f $(OPENMPI)/Makefile ] ; then $(MAKE) -C $(OPENMPI) clean; fi
	rm -f .make-$(OPENMPI) .configure-$(OPENMPI)
#	-[ -f $(OPENMPI)/Makefile ] && $(MAKE) -C $(OPENMPI) clean
#	rm -rf bin etc include sbin www doc examples lib man share logfiles

distclean: 
	rm -f .make-$(OPENMPI) .configure-$(OPENMPI)
	rm -rf $(SRC) $(TAR)
