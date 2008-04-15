include $(CBENCHOME)/make.def

export CFLAGS

SRC=lammps
TAR=$(SRC).tar.gz
URL=http://lammps.sandia.gov/tars/$(TAR)

#the date at the end of the extracted lammps source will vary depending on the release version;
# this doesn't matter for our uses
LAMMPSPATH=$(CBENCHOME)/openapps/$(SRC)/$(SRC)-*

EXECS = $(LAMMPSDIR)/src/MAKE/lmp_cbench
INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

normal: $(SRC)
	make -C -f Makefile.cbench

$(SRC): $(TAR)
	[ ! -d $(SRC) ] && tar zxf $(TAR)
	ln -sf $(LAMMPSPATH) src

$(TAR):
	$(call download-url,$(URL))
	-[ -e $(TAR) ] && touch $(TAR)

clean:
	/bin/rm -f src
	/bin/rm -rf $(LAMMPSPATH)

distclean:
	/bin/rm -rf $(TAR) $(SRC) src

install uninstall:
	$(do-$@)
