# Makefile for SHOC (Scalable Heterogeneous Computing Benchmark Suite, from ORNL) for Cbench

include ../../make.def

## check for CUDA environment
#ifeq ($(shell $(CBENCHOME)/tools/check_env.sh -cuda),1)
#  CFLAGS += -I$(CUDAINC) CXXFLAGS+=-I$(CUDAINC) CPPFLAGS+=-I$(CUDAINC) LDFLAGS+=-L$(CUDALIB)
#  CONFIGPARAMS += --with-cuda
#endif
#
## Check for OpenCL Environment
#ifeq ($(shell $(CBENCHOME)/tools/check_env.sh -opencl),1)
#  CFLAGS += -I$(OPENCLINC) CXXFLAGS += -I$(OPENCLINC) CPPFLAGS += -I$(OPENCLINC) LDFLAGS += -L$(OPENCLLIB) 
#  CONFIGPARAMS += --with-opencl
#endif

# check for CUDA environment and add flags
ifdef CUDALIB
  ifdef CUDAINC
      CFLAGS += -I$(CUDAINC)
      CXXFLAGS += -I$(CUDAINC)
      CPPFLAGS += -I$(CUDAINC)
      LDFLAGS += -L$(CUDALIB)
      LIBS += -L$(CUDALIB)
      CONFIGPARAMS += --with-cuda
  endif
endif

# Check for OpenCL Environment and add flags
ifdef OPENCLLIB
  ifdef OPENCLINC
      CFLAGS += -I$(OPENCLINC)
      CXXFLAGS += -I$(OPENCLINC)
      CPPFLAGS += -I$(OPENCLINC)
      LDFLAGS += -L$(OPENCLLIB)
      LIBS += -L$(OPENCLLIB)
      CONFIGPARAMS += --with-opencl
  endif
endif

ifndef CUDALIB
  ifndef OPENCLLIB
    $(error "Must define at least one of CUDALIB or OPENCLLIB")
  endif
endif

# these are picked up by the SHOC configure script
export MPICXX
export CFLAGS
export CXXFLAGS
export CPPFLAGS
export LDFLAGS
export LIBS

SRC=shoc-1.0.3
TAR=$(SRC).tar.gz
URL=http://ft.ornl.gov/doku/_media/shoc/$(TAR)
SHOC=$(CBENCHOME)/opensource/shoc

# NORMAL MPI COMPILE
EXECS = $(SRC)/bin/* $(SRC)/tools/driver.pl
default_tgt := all

INSTALLDIR = $(bin_dir)/shoc
INSTALLTARGETS = $(EXECS)

default: $(default_tgt)

all: $(SRC)
	@cd src; CXX=mpicxx ./configure $(CONFIGPARAMS) --with-mpi --with-mpi-includes=-I$(MPIHOME)/include --with-mpi-libraries=-L$(MPIHOME)/lib
	$(MAKE) -C $(SRC)

$(SRC): $(TAR)
	-[ ! -d $(SRC) ] && tar xzf $(TAR)
	ln -sf $(SRC) src
	touch $(SRC)

$(TAR):
	$(call download-url,$(URL))
	-[ -e $(TAR) ] && touch $(TAR)

download: $(TAR)

clean:
	-[ -d $(SRC) ] && $(MAKE) -C $(SRC) clean

distclean:
	/bin/rm -rf $(SRC) src $(TAR)

install:
	$(do-$@-shoc)

uninstall:
	$(do-$@)

