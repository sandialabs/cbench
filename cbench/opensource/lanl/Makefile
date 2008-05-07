include $(CBENCHOME)/make.def

MPICH_CH=ch_shmem

ifeq ($(CBENCH_STANDALONE),yes)
  $(warning ================ STANDALONE MPI COMPILE)
  EXECS = psnap-1.0/psnap.$(MPICH_CH)
  default_tgt := local
  MPIHOME = $(CBENCHOME)/opensource/mpich
  MPIBINNAME = bin
else
  #$(warning ================ NORMAL MPI COMPILE)
  EXECS = psnap-1.0/psnap
  default_tgt := normal
endif

INSTALLDIR = $(bin_dir)
INSTALLTARGETS = $(EXECS)

default: $(default_tgt)
normal: psnap

psnap: psnap-1.0/psnap

psnaplocal local: psnap-1.0/psnap.$(MPICH_CH)

psnap-1.0/psnap:
	$(MAKE) -C psnap-1.0

psnap-1.0/psnap.$(MPICH_CH): psnap-1.0/psnap
	/bin/mv -f psnap-1.0/psnap psnap-1.0/psnap.$(MPICH_CH)

clean:
	$(MAKE) -C psnap-1.0 clean
	/bin/rm -f $(EXECS)

smallclean:
	$(MAKE) -C psnap-1.0 clean
	
install uninstall:
	$(do-$@)
