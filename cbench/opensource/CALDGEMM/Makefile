include ../../make.def

VERSION=1.0.0
SRC=caldgemm-$(VERSION)
TAR=$(SRC).tar.bz2
URL="http://code.compeng.uni-frankfurt.de/attachments/download/8/$(TAR)"
CALDGEMMDIR="$(CBENCHOME)/opensource/CALDGEMM"

GOTOBLAS2VERSION=1.13
GOTOBLAS2SRC=GotoBLAS2-$(GOTOBLAS2VERSION)_bsd
GOTOBLAS2DIR="$(CBENCHOME)/opensource/GotoBLAS2"
GOTOBLAS2TAR="$(GOTOBLAS2DIR)/$(GOTOBLAS2SRC).tar.gz"

default: all

$(TAR):
	@echo "Downloading $(TAR)"
	$(call download-url,$(URL))

download: $(TAR)

$(SRC): $(TAR)
	@echo "Extracting $(SRC)"
	@if [ -f $(TAR) ] ; then tar -jxf $(TAR); \
	else \
		echo "$(TAR) not found, download manually or set http_proxy env variable?"; \
		exit 1; \
	fi
	ln -sf $(SRC) src

patchgoto2: $(SRC)
	@if [ ! -f $(GOTOBLAS2TAR) ]; then \
		echo "Failed to find $(GOTOBLAS2TAR) ..."; exit 1; \
	else \
		cd $(CALDGEMMDIR); \
		[ -d GotoBLAS2 ] && rm -fr $(CALDGEMMDIR)/GotoBLAS2; \
		tar -xzf $(GOTOBLAS2TAR); cd GotoBLAS2; \
		patch -p0 < ../$(SRC)/gotoblas_patch/gotoblas.patch; \
		make NO_MEMPOLICY=1 -j; \
	fi \

all: patchgoto2
	make CC=$(PLANCC) -C src 

clean:
	-[ -d $(SRC) ] && /bin/rm -fr $(SRC)

distclean: clean
	-[ -f $(TAR) ] && /bin/rm -f $(TAR) src
	-[ -d GotoBLAS2 ] && /bin/rm -fr GotoBLAS2
	-[ -L src ] && /bin/rm -f src

install: $(SRC)
	$(do-$@)

