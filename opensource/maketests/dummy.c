
#include <stdio.h>
#include <stdlib.h>

#ifdef FFTW
  #include "fftw.h"
#endif


int main(int argc, char *argv[])
{
	
	#ifdef FFTW
	ftw_print_max_memory_usage();
	#endif
}
