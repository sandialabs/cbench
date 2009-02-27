include ../../make.def

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = src/IMB-MPI1

all: src/IMB-MPI1

src/IMB-MPI1: src/make_cbench
	$(MAKE) -C src -f make_cbench all

src/make_cbench:
	cp -f make_cbench src/make_cbench

clean:
	$(MAKE) -C src -f make_cbench $@
	rm -f src/make_cbench

distclean: clean

install uninstall:
	$(do-$@)
