include $(CBENCHOME)/make.def

export CFLAGS

SRC=lammps
TAR=$(SRC).tar.gz
URL=http://lammps.sandia.gov/tars/$(TAR)

# note - this make process will strip off Lammps version (date) from the extracted source directory

EXECS = $(SRC)/src/lmp_cbench
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
	make -C $(SRC)/src clean-all

distclean:
	/bin/rm -rf $(TAR) $(SRC) src

install uninstall:
	$(do-$@)
