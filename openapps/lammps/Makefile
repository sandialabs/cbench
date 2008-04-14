include $(CBENCHOME)/make.def

export CFLAGS

SRC=lammps
TAR=$(SRC).tar.gz
URL=http://lammps.sandia.gov/tars/$(TAR)

#the date at the end of the extracted lammps source will vary depending on the release version;
# this doesn't matter for our uses
LAMMPSPATH=$(CBENCHOME)/openapps/$(SRC)/$(SRC)-*

#for use by the install script
export LAMMPSDIR=$(CBENCHOME)/openapps/$(SRC)/lammps-src
export LAMMPSMACHINE=spirit

EXECS = $(LAMMPSDIR)/src/make/lmp_$(LAMMPSMACHINE)
INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

$(SRC): $(TAR)
	[ ! -d $(SRC) ] && tar zxf $(TAR)
	ln -sf $(LAMMPSPATH) lammps-src
	@$(CBENCHOME)/openapps/lammps/install_lammps

$(TAR):
	$(call download-url,$(URL))
	-[ -e $(TAR) ] && touch $(TAR)

clean:
	/bin/rm -f $(LAMMPSDIR)
	/bin/rm -rf $(LAMMPSPATH)

distclean:
	/bin/rm -rf $(TAR) $(LAMMPSDIR)-src

install uninstall:
	$(do-$@)
