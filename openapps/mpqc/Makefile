include $(CBENCHOME)/make.def

MPQC_VERSION = 2.3.1
MPQC = mpqc-$(MPQC_VERSION)
NUM_CPUS := $(shell /usr/bin/getconf _NPROCESSORS_ONLN)

default: .make-$(MPQC)

all:

.make-$(MPQC): .configure-$(MPQC)
	(cd $(MPQC); make -j$(NUM_CPUS))
	touch $@

.configure-$(MPQC): $(MPQC)
	if [ ! -f $(MPQC)/configure ] ; then	\
		cd $(MPQC);			\
		make configure;			\
	fi
	(cd $(MPQC); ./configure --enable-threads \
	 --prefix=$(BENCH_TEST)/nodehwtest/bin/mpqc \
	 --with-cc=$(CC) --with-f77=$(F77) --with-cxx=$(CCC) \
	 --with-libs='$(BLASLIB)' )
	@touch $@

# target to untar the mpich distribution
$(MPQC):
	@echo getting source for $(MPQC) ...
	@if [ -f $(MPQC).tar.gz ] ; then				\
		zcat $(MPQC).tar.gz | tar xf -;			\
	elif [ -f $(MPQC).tar.bz2 ] ; then 			\
		bzcat $(MPQC).tar.bz2 | tar xf -;			\
	elif [ -f $(MPQC).tar ] ; then 				\
		cat $(MPQC).tar | tar xf -;			\
	else								\
		echo No $(MPQC) found;				\
		exit 1;							\
	fi

install:
	-make -C $(MPQC) install

clean:
	rm -f .make-$(MPQC) .configure-$(MPQC)
	rm -rf $(MPQC)
	rm -rf bin share

quickclean:
	make -C $(MPQC) distclean

distclean: clean
	rm -rf $(MPQC)
