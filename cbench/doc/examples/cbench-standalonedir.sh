# the required variables
export CBENCHOME=$HOME/cbench
export CBENCHSTANDALONEDIR=/tmp/cbench-test
# add the Cbench tools directory to the path to pickup
# cbench commands more easily
export PATH=$PATH:$CBENCHOME/tools
# a favorite alias for seeing what Cbench env looks like
alias showcb='env | grep --color=always -e CBENCH -e MPIHOME -e COMPILER -e RPATH -e BLASLIB -e FFTW -e LAPACK'

#
# needed if you want to build mpi w/ a different compiler
#
#export COMPILERCOLLECTION=gcc

### RECOMMENDED OPTIONS

# BLAS LIBRARY
export BLASLIB="-L$CBENCHSTANDALONEDIR/GotoBLAS2 -lgoto2 -lgfortran"

#### probably need to add in lapack and fftw variables here too
# some other environment variables we honor
#export LAPACKLIB="-L/path/to -lLAPACK"

