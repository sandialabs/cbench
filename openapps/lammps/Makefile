include $(CBENCHOME)/make.def

export CFLAGS

SRC=lammps-11Apr08
TAR=$(SRC).tar.gz
URL=http://lammps.sandia.gov/tars/$(TAR)

EXECS = $(SRC)/lmp_cbench
INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

#normal: $(SRC)
#	cd $(SRC) && ln -sf ../Makefile.cbench Makefile.cbench
#	$(MAKE) CXX=$(CXX) LINKER=$(CXX) -C $(SRC) -f Makefile.cbench

$(SRC): $(TAR)
	[ ! -d $(SRC) ] && tar zxf $(TAR)
	ln -sf $(SRC) src

$(TAR):
	$(call download-url,$(URL))
	-[ -e $(TAR) ] && touch $(TAR)

clean:
	[ -d $(SRC) ] && $(MAKE) -C $(SRC) -f Makefile.cbench clean || :

distclean:
	/bin/rm -rf $(SRC) $(TAR) src

install uninstall:
	$(do-$@)
