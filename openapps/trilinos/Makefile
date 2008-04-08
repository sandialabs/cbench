include ../../make.def

SRC=trilinos-8.0.5
TAR=$(SRC).tar.gz
URL=http://trilinos.sandia.gov/download/files/$(TAR)

BENCH_SRC=EpetraBenchmarkTest
BENCH_TAR=epetratest-0.2.tar.gz
BENCH_URL=http://www.sandia.gov/benchmarks/$(BENCH_TAR)
BENCH_EXEC=cxx_main.exe
BENCH_EXEC_RENAMED=trilinos_epetratest

EXECS = $(SRC)/EPETRA_MPI_OPT/$(BENCH_SRC)/$(BENCH_EXEC_RENAMED)
INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

# so configure_wrapper can see these
export CFLAGS
export FFLAGS
export BLASLIB
export LAPACKLIB

$(SRC)/EPETRA_MPI_OPT/$(BENCH_SRC)/$(BENCH_EXEC_RENAMED): $(SRC)
	cd $(SRC)/EPETRA_MPI_OPT && ../../configure_wrapper && $(MAKE) && $(MAKE) install
	cd $(SRC)/EPETRA_MPI_OPT/$(BENCH_SRC) && $(MAKE) && mv $(BENCH_EXEC) $(BENCH_EXEC_RENAMED)

$(SRC): $(TAR) $(BENCH_TAR)
	[ ! -d $(SRC) ] && tar zxf $(TAR)
	ln -sf $(SRC) src
	cd $(SRC) && mkdir EPETRA_MPI_OPT && cd EPETRA_MPI_OPT && tar zxf ../../$(BENCH_TAR)

$(TAR):
	wget $(URL)
	-[ -e $(TAR) ] && touch $(TAR)

$(BENCH_TAR):
	wget $(BENCH_URL)
	-[ -e $(BENCH_TAR) ] && touch $(BENCH_TAR)

clean:
	/bin/rm -rf $(SRC) src

distclean:
	/bin/rm -rf $(SRC) src $(TAR) $(BENCH_TAR)

install uninstall:
	$(do-$@)
