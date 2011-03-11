include ../../make.def

SRC=llcbench
TAR=$(SRC).tar.gz
URL="http://icl.cs.utk.edu/projects/llcbench/$(TAR)"

INSTALLDIR = $(hwtest_bin_dir)
INSTALLTARGETS = src/cachebench/cachebench

default: all

$(TAR):
	@echo "Downloading $(TAR)"
	$(call download-url,$(URL))

$(SRC): $(TAR)
	@echo "Extracting $(SRC)"
	@if [ -f $(TAR) ] ; then tar -zxf $(TAR); \
	else \
		echo "$(TAR) not found, download manually or set http_proxy env variable?"; \
		exit 1; \
	fi
	ln -sf $(SRC) src

all: $(SRC)
	$(MAKE) -C src reconfig
	$(MAKE) -C src linux-mpich
	$(MAKE) CB_CC=$(PLAINCC) CB_CFLAGS="$(CFLAGS)" -C src/cachebench cachebench
#	$(MAKE) BB_CC=$(PLAINCC) BB_F77=$(PLAINF77) BB_LD=$(PLAINF77) BB_LIBS="$(BLASLIB) -lrt" CB_CC=$(PLAINCC) MP_MPI_CC=$(CC) -C src compile

clean:
	$(MAKE) -C src clean; $(MAKE) -C src/cachebench clobber

distclean:
	-/bin/rm -rf $(SRC) $(TAR) .patched src

install uninstall:
	$(do-$@)

