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

/* 
 * Marcus R. Epperson 2005   mrepper@sandia.gov
 * NWCC test for RS5
 * Simple mpi_sleep test so that 60 jobs can be submitted at once, and then
 * clean themselves up after the time has elapsed.
 *
 * Run: mpiexec -np 1 ./rs5 600  # 10 minutes sleep time, for example
 *      (repeat 60 times)
 */
#include <stdio.h>
#include <stdlib.h>  /* strtoul */
#include <limits.h>  /* ULONG_MAX */
#include <time.h>    /* nanosleep */
#include <mpi.h>

#define mpicall(f, args...) do {                                  \
                               if (f(args) != MPI_SUCCESS) {      \
                                   fprintf(stderr, #f" error\n"); \
                                   fflush(stderr);                \
                                   MPI_Abort(MPI_COMM_WORLD, 1);  \
                               }                                  \
                            } while (0);

int main (int argc, char **argv) {
    int rank, size;
    unsigned long wait;
    struct timespec ts;

    mpicall(MPI_Init, &argc, &argv);
    mpicall(MPI_Comm_size, MPI_COMM_WORLD, &size);
    mpicall(MPI_Comm_rank, MPI_COMM_WORLD, &rank);

    if ( rank == 0 ) {
        if ( argc < 2 ) {
            fprintf(stderr, "Usage: %s <wait in seconds>\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, 1);
            exit(1);
        }

        wait = strtoul( argv[1], NULL, 10 );
        if ( wait == ULONG_MAX ) {
            perror("strtoul");
            MPI_Abort(MPI_COMM_WORLD, 1);
            exit(1);
        }

    }

    /* prepost receive from n-1 node */
    if ( rank != 0 )
        mpicall(MPI_Recv, &wait, 1, MPI_UNSIGNED_LONG, rank-1, MPI_ANY_TAG,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    ts.tv_sec  = (time_t)wait;
    ts.tv_nsec = 0;

    printf("node %4d ready to sleep for %lu seconds\n", rank, wait);
    fflush(stdout);

    /* send message to n+1 node */
    if ( rank != (size - 1) )
        mpicall(MPI_Send, &wait, 1, MPI_UNSIGNED_LONG, rank+1, 1, MPI_COMM_WORLD);

    mpicall(MPI_Barrier, MPI_COMM_WORLD);

    if ( rank == 0 ) {
        printf("Sleeping...  ");
        fflush(stdout);
    }

    nanosleep(&ts, NULL);

    mpicall(MPI_Barrier, MPI_COMM_WORLD);

    if ( rank == 0 ) {
        printf("Done.\n");
        fflush(stdout);
    }

    MPI_Finalize();

    return 0;
}
