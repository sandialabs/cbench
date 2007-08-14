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
 * MPI program to allocate 1.9 GB on rank 0 and send it to rank 1 COUNT times.
 * For NWCC requirement MP3.
 *
 * NOTE: Run with ppn=1 or you'll not have enough memory
 *
 * Marcus R. Epperson - 10/2005
 */
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <mpi.h>

double mysecond(void);

#define MB (1024UL*1024UL)

#define COUNT 3   /* number of times to send the message */
#define WAIT 5    /* delay between each send, in seconds */
#define DEFAULT_SPACE (1946UL)  /* the size in MB to send each time (1.9 GB rounded up) */
//#define DEFAULT_SPACE (800UL)  // testing

/*
 * All MPI calls wrapped with this so the code doesn't need return value
 * checking everywhere.  Used for all MPI calls except MPI_Abort and
 * MPI_Finalize.
 */
#define mpicall(f, args...) do {                                  \
                               if (f(args) != MPI_SUCCESS) {      \
                                   fprintf(stderr, #f" error\n"); \
                                   fflush(stderr);                \
                                   MPI_Abort(MPI_COMM_WORLD, 1);  \
                               }                                  \
                            } while (0);

int main (int argc, char **argv) {
        int rank, size;
        int i;
        double start, end;
        char *chunk;
        unsigned long megs = DEFAULT_SPACE;

        mpicall(MPI_Init, &argc, &argv);
        mpicall(MPI_Comm_size, MPI_COMM_WORLD, &size);
        mpicall(MPI_Comm_rank, MPI_COMM_WORLD, &rank);

        if ( rank == 0 && size != 2 ) {
                fprintf(stderr, "Error: Requires exactly 2 processes.\n");
                fflush(stderr);
                MPI_Abort(MPI_COMM_WORLD, 1);
                return 0;
        }

        mpicall(MPI_Barrier, MPI_COMM_WORLD);

        if ( rank == 0 ) {
                printf("Allocating %lu MB... ", megs);
                fflush(stdout);
        }

        chunk = calloc( megs * MB, sizeof *chunk );

        if ( chunk == NULL ) {
                fprintf(stderr, "Error: Could not allocate %lu MB on rank %d (errno %d)\n", megs, rank, errno);
                fflush(stderr);
                MPI_Abort(MPI_COMM_WORLD, 1);
                return -1;
        }

        if ( rank == 0 ) {
                memset(chunk, 0xa, megs * MB);
                printf("success!\n");
        }

        mpicall(MPI_Barrier, MPI_COMM_WORLD);

        for ( i = 1 ; i <= COUNT ; i++ ) {
                mpicall(MPI_Barrier, MPI_COMM_WORLD);
                start = mysecond();

                if ( rank == 0 ) {
                        printf("Pass %d... ", i);
                        mpicall(MPI_Send, chunk, megs * MB, MPI_CHAR, 1, 1, MPI_COMM_WORLD);
                } else {
                        mpicall(MPI_Recv, chunk, megs * MB, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                        /* XXX: should have more verification than this, but I'm lazy */
                        if ( chunk[0] != 0xa || chunk[100] != 0xa )
                                fprintf(stderr, "Error: data not received correctly\n");
                }

                mpicall(MPI_Barrier, MPI_COMM_WORLD);
                end = mysecond();

                if ( rank == 0 ) {
                        printf("success!  (elapsed time = %f seconds)\n", end - start);
                        if ( i != COUNT )
                                printf("Sleeping for %d seconds.\n", WAIT);
                }

                if ( i != COUNT )
                        sleep(WAIT);
        }

        if ( rank == 0 )
                printf("Done.\n");

        mpicall(MPI_Barrier, MPI_COMM_WORLD);

        free(chunk);
        MPI_Finalize();

        return 0;
}
