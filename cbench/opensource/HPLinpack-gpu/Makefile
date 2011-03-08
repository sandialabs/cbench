include ../../make.def

VERSION=1.0.1
SRC=hpl-gpu-$(VERSION)
TAR=$(SRC).tar.bz2
URL="http://code.compeng.uni-frankfurt.de/attachments/download/15/$(TAR)"
HPLGPU=$(CBENCHOME)/opensource/HPLinpack-gpu


install: $(SRC)
	$(do-$@)

$(TAR):
	@if [ ! -e $(TAR) ]; then $(call download-sf-url,$(URL)); fi

$(SRC): $(TAR)
	-[ ! -d $(SRC) ] && tar jxf $(TAR)
	ln -sf $(SRC) src
	ln -sf $(HPLGPU)/Make.cbench $(HPLGPU)/$(SRC)/Make.cbench
	touch $(SRC)

linpack: $(SRC)
	$(MAKE) -C $(SRC) arch=cbench

clean:
	-[ -d $(SRC) ] && /bin/rm -fr $(SRC)

distclean: clean
	-[ -f $(TAR) ] && /bin/rm -f $(TAR) 
	-[ -L src ] && /bin/rm -f src

