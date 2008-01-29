# the required variables
export CBENCHOME=$HOME/cbench
export CBENCHTEST=$HOME/cbench-test
export COMPILERCOLLECTION=intel
export MPIHOME=/apps/openmpi

# optional but usually very helpful variables
export RPATH=/projects/global/x86_64/libraries/goto_blas
export BLASLIB="-Wl,-rpath,$RPATH -L$RPATH -lgoto -lpthread -lm"

# some other environment variables we honor
#export LAPACKLIB="-L/path/to -lLAPACK"
#export CBENCH_OPTFLAGS="-O1 -xP"

