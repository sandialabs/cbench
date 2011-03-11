include ../../make.def

#SRC=stress-0.18.9
VERSION=1.0.4
SRC=stress-$(VERSION)
TAR=$(SRC).tar.gz
URL="http://weather.ou.edu/~apw/projects/stress/$(TAR)"

EXEC = $(SRC)/src/stress
INSTALLDIR = $(hwtest_bin_dir)
INSTALLTARGETS = $(EXEC)


$(EXEC): $(SRC) .patched
	cd $(SRC) && ./configure
	$(MAKE) -C $(SRC)

clean: 
	-$(MAKE) -C $(SRC) clean

distclean:
	-/bin/rm -rf $(SRC) $(TAR) .patched src

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

.patched:
	for patch in *.patch ; do \
           cd $(SRC); \
           echo "Applying patch [ $$patch ]"; \
           patch -p1 <../$$patch; \
           cd - ; \
        done
	-touch .patched

install uninstall:
	$(do-$@)


