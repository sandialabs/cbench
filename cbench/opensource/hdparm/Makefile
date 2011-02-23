include ../../make.def

VERSION=9.37
SRC=hdparm-$(VERSION)
TAR=$(SRC).tar.gz
URL="http://sourceforge.net/projects/hdparm/files/hdparm/$(TAR)"

install: build
	$(do-$@)

build: $(TAR)
	@if [ ! -d $(SRC) ]; then tar zxf $(TAR); fi
	cd $(SRC); CC=$(PLAINCC) make

$(TAR):
	@if [ ! -e $(TAR) ]; then $(call download-sf-url,$(URL),$(TAR)); fi

clean:
	-[ -d $(SRC) ] && /bin/rm -fr $(SRC)

distclean: clean
	-[ -f $(TAR) ] && /bin/rm -f $(TAR) 


