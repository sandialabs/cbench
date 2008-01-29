# the required variables
setenv CBENCHOME /scratch4/cdmaest/svn/cbench
setenv CBENCHTEST /scratch4/cdmaest/svn/cbench-test
setenv COMPILERCOLLECTION intel
setenv MPIHOME /apps/openmpi

# optional but usually very helpful variables
setenv RPATH /projects/global/x86_64/libraries/goto_blas
setenv BLASLIB "-Wl,-rpath,$RPATH -L$RPATH -lgoto -lpthread -lm"

# some other environment variables we honor
#setenv LAPACKLIB "-L/path/to -lLAPACK"
#setenv CBENCH_OPTFLAGS "-O1 -xP"

