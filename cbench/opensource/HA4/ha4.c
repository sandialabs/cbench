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
 * MPI program to allocate 0.95 GB per process, for NWCC requirement HA4
 * Marcus R. Epperson - 9/2005
 */
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <mpi.h>

/* proto for meminfo (meminfo.c) */
int meminfo(unsigned long long *, unsigned long long *);

#define SLEEP 5
#define MB (1024UL*1024UL)

/*
 * DEFAULT_SPACE: the size (in MB) to request from the system via malloc
 */
//#define DEFAULT_SPACE (950UL)      // 0.95 "GB"
//#define DEFAULT_SPACE (972UL)      // 0.95 GB rounded down
#define DEFAULT_SPACE (973UL)      // 0.95 GB rounded up

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
        char *chunk;
        long i;
        long pagesize = 0;
        unsigned long megs = DEFAULT_SPACE;
        unsigned long long mem = 0;
        unsigned long long swap_used = 0;
        int ret;

        mpicall(MPI_Init, &argc, &argv);
        mpicall(MPI_Comm_size, MPI_COMM_WORLD, &size);
        mpicall(MPI_Comm_rank, MPI_COMM_WORLD, &rank);

        // mre 2005/12/12  - user can override memory footprint
        if (argc > 1)
            megs = strtoul(argv[1], NULL, 10);

        pagesize = sysconf(_SC_PAGESIZE);

        // mre 2005/12/12  - user can override "pagesize"
        if (argc > 2)
            pagesize = strtol(argv[2], NULL, 10);

        if ( pagesize == 0 || pagesize == -1 ) {
                fprintf(stderr, "Cannot determine pagesize on rank %d.  Every byte in array will be touched.\n", rank);
                fflush(stderr);
                pagesize = 1;
        } else if ( pagesize != 4096 ) {
                printf("unusual pagesize (%ld) on rank %d\n", pagesize, rank);
        }

        if ( rank == 0 ) {
                printf("Allocating %lu MB in each of %d processes... ", megs, size);
                fflush(stdout);
        }

        mpicall(MPI_Barrier, MPI_COMM_WORLD);

        chunk = malloc( megs * MB * sizeof *chunk );
        if ( chunk == NULL ) {
                fprintf(stderr, "Error: Could not allocate %lu MB on rank %d (errno %d)\n", megs, rank, errno);
                fflush(stderr);
                MPI_Abort(MPI_COMM_WORLD, 1);
                return -1;
        }

        mpicall(MPI_Barrier, MPI_COMM_WORLD);

        if ( rank == 0 ) {
                printf("success!\nTouching all pages... ");
                fflush(stdout);
        }

        /* skip pagesize bytes at a time */
        for( i = 0 ; i < (megs*MB) ; i += pagesize )
                chunk[i] = 0xaa;

        mpicall(MPI_Barrier, MPI_COMM_WORLD);

        if ( rank == 0 ) {
                printf("success!\nSleeping for %d seconds... ", SLEEP);
                fflush(stdout);
        }

        sleep(SLEEP);

        mpicall(MPI_Barrier, MPI_COMM_WORLD);

        if ( rank == 0 ) {
                printf("Done.\n");
                fflush(stdout);
        }

        /* check total memory and used swap on each node */
        if ( (ret = meminfo(&mem, &swap_used)) ) {
                fprintf(stderr, "Error: meminfo returned badly (%d) on rank %d\n", ret, rank);
                MPI_Abort(MPI_COMM_WORLD, 1);
                return -2;
        }

        mpicall(MPI_Barrier, MPI_COMM_WORLD);

        printf("Rank %2d - Total Mem: %llu kB - Swap Used: %llu kB\n", rank, mem, swap_used);
        fflush(stdout);

        free(chunk);
        MPI_Finalize();

        return 0;
}
