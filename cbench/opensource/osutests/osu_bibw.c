
/*
 * Copyright (C) 2002-2005 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.  
 */

#include "mpi.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#define MAX_REQ_NUM 1000

#define MAX_ALIGNMENT 16384

#ifndef MAX_MSG_SIZE
#define MAX_MSG_SIZE (1<<22)
#endif

#define MYBUFSIZE (MAX_MSG_SIZE + MAX_ALIGNMENT)

int loop = 100;
int window_size = 64;
int skip = 10;

int loop_large = 20;
int window_size_large = 64;
int skip_large = 2;

int large_message_size = 8192;

char s_buf1[MYBUFSIZE];
char r_buf1[MYBUFSIZE];

MPI_Request send_request[MAX_REQ_NUM];
MPI_Request recv_request[MAX_REQ_NUM];
MPI_Status  reqstat[MAX_REQ_NUM];

int main(int argc, char *argv[])
{

    int myid, numprocs, i, j;
    int size, align_size;
    char *s_buf, *r_buf;
    double t_start = 0.0, t_end = 0.0, t = 0.0;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    align_size = getpagesize();
    s_buf =
        (char *) (((unsigned long) s_buf1 + (align_size - 1)) /
                  align_size * align_size);
    r_buf =
        (char *) (((unsigned long) r_buf1 + (align_size - 1)) /
                  align_size * align_size);

    if (myid == 0) {
        fprintf(stdout,
                "# OSU MPI Bidirectional Bandwidth Test (Version 2.0)\n");
        fprintf(stdout, "# Size\t\tBi-Bandwidth (MB/s) \n");
    }

    for (size = 1; size <= MAX_MSG_SIZE; size *= 2) {

        /* touch the data */
        for (i = 0; i < size; i++) {
            s_buf[i] = 'a';
            r_buf[i] = 'b';
        }

        if (size > large_message_size) {
            loop = loop_large;
            skip = skip_large;
            window_size = window_size_large;
        }

        if (myid == 0) {
            for (i = 0; i < loop + skip; i++) {

                if (i == skip)
                    t_start = MPI_Wtime();
                for (j = 0; j < window_size; j++)
                    MPI_Irecv(r_buf, size, MPI_CHAR, 1, 10,
                              MPI_COMM_WORLD, recv_request + j);
                for (j = 0; j < window_size; j++)
                    MPI_Isend(s_buf, size, MPI_CHAR, 1, 100,
                              MPI_COMM_WORLD, send_request + j);
                MPI_Waitall(window_size, send_request, reqstat);
                MPI_Waitall(window_size, recv_request, reqstat);

            }

            t_end = MPI_Wtime();
            t = t_end - t_start;

        } else if (myid == 1) {

            for (i = 0; i < loop + skip; i++) {

                for (j = 0; j < window_size; j++)
                    MPI_Irecv(r_buf, size, MPI_CHAR, 0, 100,
                              MPI_COMM_WORLD, recv_request + j);
                for (j = 0; j < window_size; j++)
                    MPI_Isend(s_buf, size, MPI_CHAR, 0, 10,
                              MPI_COMM_WORLD, send_request + j);
                MPI_Waitall(window_size, send_request, reqstat);
                MPI_Waitall(window_size, recv_request, reqstat);

            }

        }

        if (myid == 0) {
            double tmp;
            tmp = ((size * 1.0) / 1.0e6) * loop * window_size * 2;
            fprintf(stdout, "%d\t\t%f\n", size, tmp / t);
        }

    }

    MPI_Finalize();
    return 0;

}
