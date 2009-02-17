include ../../make.def

export CFLAGS

SRC=phloem-1.0.0
TAR=phloem_v1.0.tgz
URL=https://asc.llnl.gov/sequoia/benchmarks/$(TAR)

EXECS = $(SRC)/linktest*/linux/linktest $(SRC)/presta*/com $(SRC)/sqmr*/sqmr $(SRC)/torustest*/linux/{torustest,generate} $(SRC)/mpiGraph*/{mpiGraph,crunch_mpiGraph} $(SRC)/mpiBench*/{mpiBench_Allreduce,mpiBench_Barrier,mpiBench_Bcast,crunch_mpiBench}
INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

CFLAGS += -DPRINT_ENV

all normal: $(SRC)
	$(MAKE) CC=$(CC) -C $(SRC)

$(SRC): $(TAR)
	[ ! -d $(SRC) ] && tar zxf $(TAR)
	ln -sf $(SRC) src

$(TAR) download:
	$(call download-url,$(URL))
	-[ -e $(TAR) ] && touch $(TAR)

clean:
	[ -d $(SRC) ] && $(MAKE) -C $(SRC) clean || :

distclean:
	/bin/rm -rf $(SRC) $(TAR) src

install uninstall:
	$(do-$@)
