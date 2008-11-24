include ../../make.def

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = src/current/iozone

ifeq ($(ARCH),x86_64)
	TARGET=linux-AMD64
else
	TARGET=linux
endif

ifeq ($(COMPILER),intel)
	LDFLAGS += -i-static -static-libcxa -static
endif

default all:
	$(MAKE) CC=$(PLAINCC) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" -C src/current $(TARGET)

clean:
	$(MAKE) -C src/current clean

install uninstall:
	$(do-$@)
