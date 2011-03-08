include ../../make.def

VERSION=1.0.1
SRC=hpl-gpu-$(VERSION)
TAR=$(SRC).tar.bz2
URL="http://code.compeng.uni-frankfurt.de/attachments/download/15/$(TAR)"

install: build
	$(do-$@)

build: $(TAR)
	@if [ ! -d $(SRC) ]; then tar jxf $(TAR); fi

$(TAR):
	@if [ ! -e $(TAR) ]; then $(call download-sf-url,$(URL)); fi

clean:
	-[ -d $(SRC) ] && /bin/rm -fr $(SRC)

distclean: clean
	-[ -f $(TAR) ] && /bin/rm -f $(TAR) 


