# the required variables
export CBENCHSTANDALONEDIR=$HOME/cbench-test
export CBENCHOME=$HOME/svn/cbench

export PATH=$CBENCHSTANDALONEDIR/openmpi/bin:$PATH
export LD_LIBRARY_PATH=$CBENCHSTANDALONEDIR/GotoBLAS2:$CBENCHSTANDALONEDIR/openmpi/lib:$LD_LIBRARY_PATH
export BLASLIB="-L$CBENCHSTANDALONEDIR/GotoBLAS2 -lgoto2 -lgfortran -lm"
export LAPACKLIB="-L$CBENCHSTANDALONEDIR/GotoBLAS2 -lgoto2"

# add the Cbench tools directory to the path to pickup
# cbench commands more easily
export PATH=$PATH:$CBENCHOME/tools

# a favorite alias for seeing what Cbench env looks like
alias showcb='env | grep --color=always -e CBENCH -e MPIHOME -e COMPILER -e RPATH -e BLASLIB -e FFTW -e LAPACK'

