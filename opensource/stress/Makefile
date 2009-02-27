include ../../make.def

SRC=stress-0.18.9
TAR=$(SRC).tar.gz

EXEC = $(SRC)/src/stress
INSTALLDIR = $(hwtest_bin_dir)
INSTALLTARGETS = $(EXEC)


$(EXEC): $(SRC) .patched
	cd $(SRC) && ./configure
	$(MAKE) -C $(SRC)

clean: 
	-$(MAKE) -C $(SRC) clean

distclean:
	-/bin/rm -rf $(SRC) .patched src

$(SRC): $(TAR)
	[ -d $(SRC) ] || tar zxf $(TAR) && rm -f .patched
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
