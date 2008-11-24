include ../../make.def

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = src/IMB-MPI1

default all: src/make_cbench
	$(MAKE) -C src -f make_cbench $@

src/make_cbench:
	cp -f make_cbench src/make_cbench

clean:
	$(MAKE) -C src -f make_cbench $@
	rm -f src/make_cbench

install uninstall:
	$(do-$@)
