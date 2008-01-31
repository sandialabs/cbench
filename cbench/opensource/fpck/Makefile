include ../../make.def

CFLAGS = $(COMMON_FLAGS) -O1

EXEC = fpck
OBJS = fpck.o meminfo.o
INSTALLDIR = $(hwtest_bin_dir)
INSTALLTARGETS = $(EXEC)

all: shared

shared: $(EXEC)

$(EXEC): $(OBJS)
	$(PLAINCC) -o $(EXEC) $(OBJS)

static: $(OBJS)
	$(PLAINCC) -static -o $(EXEC) $(OBJS)
	strip $(EXEC)

%.o: %.c
	$(PLAINCC) $(CFLAGS) -c $<

clean:
	rm -f $(EXEC) $(OBJS)

install uninstall:
	$(do-$@)
