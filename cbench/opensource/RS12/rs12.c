/*
###############################################################################
#    Copyright (2005) Sandia Corporation.  Under the terms of Contract
#    DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
#    certain rights in this software
#
#    This file is part of Cbench.
#
#    Cbench is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    any later version.
#
#    Cbench is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with Cbench; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
###############################################################################
*/

#include <stdio.h>
#include <unistd.h>     // exec*, fork
#include <sys/types.h>  // fork
#include <sys/wait.h>   // wait
#include <mpi.h>

int main (int argc, char **argv) {
    int rank, size, ret;
    char file[100];

    if ( MPI_Init(&argc, &argv) != MPI_SUCCESS ) {
        fprintf(stderr, "MPI initialization error\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //strncpy(argv[0], "/bin/ls", strlen(argv[0])+1);    // assumes len(argv[0]) >= len("/bin/ls")
    //ret = execv("/bin/ls", argv);
    //or...

    /* try to ls a file called /n where n is this process's rank */
    sprintf(file, "/%d", rank);
    ret = execl("/bin/ls", "/bin/ls", file, NULL);
    fprintf(stderr, "rank %d: bad return from exec: %d\n", rank, ret);
    perror("execv");

    MPI_Finalize();

    return 0;
}
