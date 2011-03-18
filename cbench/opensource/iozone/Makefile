include ../../make.def

VERSION=3_373
SRC=iozone$(VERSION)
TAR=$(SRC).tar
URL="http://www.iozone.org/src/current/$(TAR)"

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = src/src/current/iozone src/src/current/fileop

ifeq ($(ARCH),x86_64)
	TARGET=linux-AMD64
else
	TARGET=linux
endif

ifeq ($(COMPILER),intel)
	LDFLAGS += -i-static -static-libcxa -static
endif

default: src/src/current/iozone src/src/current/fileop

src/src/current/iozone src/src/current/fileop:
	$(MAKE) all

all: $(SRC)
	$(MAKE) CC=$(PLAINCC) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" -C src/src/current $(TARGET)


$(TAR):
	@echo "Downloading $(TAR)"
	$(call download-url,$(URL))

download: $(TAR)

$(SRC): $(TAR)
	@echo "Extracting $(SRC)"
	@if [ -f $(TAR) ] ; then tar -xf $(TAR); \
	else \
		echo "$(TAR) not found, download manually or set http_proxy env variable?"; \
		exit 1; \
	fi
	ln -sf $(SRC) src
	patch -p0 < make.patch

clean:
	$(MAKE) -C src/src/current clean

distclean:
	-/bin/rm -rf $(SRC) $(TAR) src

install uninstall:
	$(do-$@)

