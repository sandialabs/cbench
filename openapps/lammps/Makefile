include $(CBENCHOME)/make.def

export CFLAGS

SRC=lammps
TAR=$(SRC).tar.gz
URL=http://lammps.sandia.gov/tars/$(TAR)

#the date at the end of the extracted lammps source will vary depending on the release version;
# this doesn't matter for our uses
LAMMPSPATH=$(CBENCHOME)/openapps/$(SRC)/$(SRC)-*

EXECS = $(SRC)/src/MAKE/lmp_cbench
INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

all: $(SRC)
	make -C $(SRC)/src cbench

$(SRC): $(TAR)
	[ ! -d $(SRC) ] && tar zxf $(TAR)
	mv lammps-* $(SRC)
	ln -sf $(SRC) src
	cd $(SRC)/src/MAKE; ln -sf ../../../Makefile.cbench .
	touch $(SRC)

$(TAR):
	$(call download-url,$(URL))
	-[ -e $(TAR) ] && touch $(TAR)

clean:
	make -C $(SRC)/src clean

distclean:
	/bin/rm -rf $(TAR) $(SRC) src

install uninstall:
	$(do-$@)
