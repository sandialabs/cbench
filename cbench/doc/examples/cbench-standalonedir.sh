# the required variables
export CBENCHOME=$HOME/cbench
export CBENCHSTANDALONEDIR=$HOME/cbench-test

#
# needed if you want to build mpi w/ a different compiler
#
#export COMPILERCOLLECTION=gcc

export LD_LIBRARY_PATH=$CBENCHSTANDALONEDIR/GotoBLAS2:$LD_LIBRARY_PATH
export BLASLIB="-L$CBENCHSTANDALONEDIR/GotoBLAS2 -lgoto2 -lgfortran"
export LAPACKLIB="-L$CBENCHSTANDALONEDIR/GotoBLAS2 -lgoto2"

# add the Cbench tools directory to the path to pickup
# cbench commands more easily
export PATH=$PATH:$CBENCHOME/tools

# a favorite alias for seeing what Cbench env looks like
alias showcb='env | grep --color=always -e CBENCH -e MPIHOME -e COMPILER -e RPATH -e BLASLIB -e FFTW -e LAPACK'

