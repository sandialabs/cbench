include ../../make.def

#SRC=stress-0.18.9
VERSION=1.0.4
SRC=stress-$(VERSION)
TAR=$(SRC).tar.gz
URL="http://weather.ou.edu/~apw/projects/stress/$(TAR)"

EXEC = $(SRC)/src/stress
INSTALLDIR = $(hwtest_bin_dir)
INSTALLTARGETS = $(EXEC)

default: $(EXEC)

download: $(TAR)

$(TAR):
	$(call download-curl-url,$(URL))


$(SRC): $(TAR)
	$(call untar-src,$(TAR))
	ln -sf $(SRC) src

.patched:
	for patch in *.patch ; do \
           cd $(SRC); \
           echo "Applying patch [ $$patch ]"; \
           patch -p1 <../$$patch; \
           cd - ; \
        done
	-touch .patched

$(EXEC): $(SRC) .patched
	cd $(SRC) && ./configure > ../.configure-$(SRC) 2>&1
	$(MAKE) -C $(SRC) > .make-$(SRC) 2>&1

clean: 
	$(MAKE) -C $(SRC) clean

distclean:
	/bin/rm -rf $(SRC) $(TAR) .patched src .make-$(SRC) .configure-$(SRC)

install uninstall:
	$(do-$@)
