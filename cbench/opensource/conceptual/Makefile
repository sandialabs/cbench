include ../../make.def

VERSION=1.2
SRC=conceptual-$(VERSION)
TAR=$(SRC).tar.gz
URL="http://sourceforge.net/projects/conceptual/files/conceptual/$(VERSION)/$(TAR)"

$(SRC) build: download
	-[ ! -d $(SRC) ] && tar zxf $(TAR)
	cd $(SRC); MPICC=$(MPIHOME)/bin/mpicc ./configure --prefix=$(CBENCHTEST);

$(TAR) download:
	$(call download-sf-url,$(URL),$(TAR))
	-[ -e $(TAR) ] && touch $(TAR)

clean:
	-[ -d $(SRC) ] && /bin/rm -fr $(SRC)

distclean: clean
	-[ -f $(TAR) ] && /bin/rm -f $(TAR) 

