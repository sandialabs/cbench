include $(CBENCHOME)/make.def

SRC=irs.1.0
TAR=irs_v1.0.tar.gz
URL=https://asc.llnl.gov/sequoia/benchmarks/$(TAR)


EXECS = build.space/codes_opt/irs
INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

all: $(SRC)
	export PATH=$(PWD)/src/scripts:$(PATH); \
	  make CC=$(CC) CFLAGS="$(CFLAGS) $(OPENMP_ENABLE)" COMMON_FLAGS="$(COMMON_FLAGS) $(OPENMP_ENABLE)" -C build.space/build depend
	export PATH=$(PWD)/src/scripts:$(PATH); \
	  make CC=$(CC) CFLAGS="$(CFLAGS) $(OPENMP_ENABLE)" COMMON_FLAGS="$(COMMON_FLAGS) $(OPENMP_ENABLE)" -C build.space/build

$(SRC) src: $(TAR)
	[ ! -d $(SRC) ] && tar zxf $(TAR)
	ln -sf $(SRC) src
	cd $(SRC)/build/arch; ln -sf ../../../make.cbench .
	touch $(SRC)
	perl src/scripts/irs_build build.space $(SRC)
	cd build.space/build; perl ../../$(SRC)/scripts/irs_config cbench
	cd $(SRC)/scripts; sed -i 's/local\/bin\/perl/bin\/perl/' irs_*
	cd $(SRC)/scripts; patch -p0 < ../../irs_hrcreate.patch

$(TAR) download:
	$(call download-url,$(URL))
	-[ -e $(TAR) ] && touch $(TAR)

clean:
	export PATH=$(PWD)/src/scripts:$(PATH); \
	  make CC=$(CC) CFLAGS="$(CFLAGS) $(OPENMP_ENABLE)" COMMON_FLAGS="$(COMMON_FLAGS) $(OPENMP_ENABLE)" -C build.space/build clean

distclean:
	/bin/rm -rf $(TAR) $(SRC) src build.space

install uninstall:
	$(do-$@)
