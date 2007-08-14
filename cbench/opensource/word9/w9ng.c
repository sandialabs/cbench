#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <signal.h>
#include <unistd.h>
#include "mpi.h"

#ifdef USE_LONG
#define DTYPE       long
#define MPI_DTYPE   MPI_LONG
#else
#define DTYPE       int
#define MPI_DTYPE   MPI_INT
#endif

#define RAND_MOD    1000
#define COOKIE      8301965


/* getopt related variables */
int ch;
extern char *optarg;
extern int optind;

int rank;
int nproc;
volatile int shutdown = 0;
int verbose = 1;
int one_malloc = 0;
int MAXBUF;
int msg_size = 3;
int num_errors = 0;
int msg_count = 0;
int mpi_behavior = 0;
int align_offset = 0;   /* i.e. default to aligned to malloced buf! */


/* forward declarations */
void handle_signals(int sig);
DTYPE random_count();
void usage(char *);


main(int argc, char **argv)
{
    DTYPE **the_matrix;
    DTYPE *send_buf_base;
    DTYPE *send_buf;
    DTYPE *recv_buf_base;
    DTYPE *recv_buf;
    DTYPE *rcv;
    DTYPE ***all_matrices = NULL;
    DTYPE *send_matrix = NULL;
    int i, j, k, m;
    int nsend;
    int nrecv;
    int nneigh;
    int irecv;
    int isend = 0;
    int size;
    int ioffset;
    int ierror;
    int iter;
    int count;
    MPI_Request *request = NULL;
    MPI_Status *status = NULL;
    MPI_Request *mpi_requests = NULL;
    int max_nsend = 0;
    int max_nrecv = 0;
    int x,y;
    int master;
    int slave;
    MPI_Status mpi_status;
    int recv_units;
    unsigned long temp_ulong;
    int nrecv_size;

    /*
    mallopt(M_MMAP_MAX,0);
    mallopt(MALLOC_TRIM_THRESHOLD,-1);
    */

    ierror = MPI_Init(&argc,&argv);

    ierror = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    ierror = MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    /* parse the command line */
    while ((ch= getopt(argc, argv, "a:b:c:m:ov")) != EOF)   {
        switch (ch) {
            case 'a':
                align_offset = atoi(optarg);
                break;
            case 'c':
                msg_count = atoi(optarg);
                break;
            case 'o':
                one_malloc = 1;
                break;
            case 'm':
                msg_size = atoi(optarg);
                break;
            case 'v':
                verbose++;
                break;
            case 'b':
                mpi_behavior = atoi(optarg);
                break;
            default:
                usage(argv[0]);
                exit(-1);
        }
    }

    /* finish some variable inits */
    MAXBUF = nproc * (RAND_MOD+1) * msg_size;
    
    if (align_offset != 0) {
        one_malloc = 1;
        msg_count = 0;
    }
    
    /* install signal handlers */
    signal(SIGUSR1,handle_signals);
    signal(SIGUSR2,handle_signals);
    /*signal(SIGTERM,handle_signals);*/
    
    the_matrix = (DTYPE **) malloc(nproc*sizeof(DTYPE *));
    for (i = 0; i < nproc; i++) {
       the_matrix[i] = (DTYPE *) malloc(nproc*sizeof(DTYPE));
    }
    request = (MPI_Request *) malloc(nproc*sizeof(MPI_Request));
    mpi_requests = (MPI_Request *) malloc(nproc*sizeof(MPI_Request));
    status = (MPI_Status *) malloc(nproc*sizeof(MPI_Status));
    rcv = (DTYPE *) malloc(nproc*sizeof(DTYPE));

    if (one_malloc) {
        send_buf_base = (DTYPE *) malloc(MAXBUF*sizeof(DTYPE));
        send_buf = (DTYPE*)((long)send_buf_base + align_offset);
        nrecv_size = MAXBUF*sizeof(DTYPE);
        recv_buf_base = (DTYPE *) malloc(nrecv_size);
        recv_buf = (DTYPE*)((long)recv_buf_base + align_offset);;
        if (1) {
            printf("%d send_buf: %d messages w/ %d bytes @ %p (base %p)\n",rank,nproc*RAND_MOD,
                   MAXBUF*sizeof(DTYPE),send_buf,send_buf_base);
            printf("%d recv_buf: %d messages w/ %d bytes @ %p (base %p)\n",rank,nproc*RAND_MOD,
                   MAXBUF*sizeof(DTYPE),recv_buf,recv_buf_base);
        }
    }

    /* print out config info */
    if (rank == 0 && verbose) {
        printf("%d: sizeof(DTYPE) = %d bytes\n",rank,sizeof(DTYPE));
        printf("%d: one_malloc = %d\n",rank,one_malloc);
        printf("%d: msg_size = %d\n",rank,msg_size);
        printf("%d: msg_count = %d\n",rank,msg_count);
        printf("%d: maxbuf = %d bytes\n",rank,MAXBUF);
        printf("%d: align_offset = %d bytes\n",rank,align_offset);
        printf("%d: mpi_behavior = %d\n",rank,mpi_behavior);
        printf("%d: MPI msg size guesstimate = %d bytes\n",rank,
               random_count() * msg_size * sizeof(DTYPE));
    }
    
    
    iter = 0;
    while (!shutdown) {

       if (rank == 0 && iter%10000 == 0) printf("iter %d\n", iter);
       iter++;

       for (i = 0; i < nproc; i++) {
         for (j = 0; j < nproc; j++) {
            if (i != j)
               the_matrix[i][j] = random_count();
            else
               the_matrix[i][j] = 0;
         }
       } 

       for (i = nsend = nrecv = 0; i < nproc; i++) {
          nsend += the_matrix[rank][i];
          nrecv += the_matrix[i][rank];
       }

#if 0      
       if (nsend > max_nsend) {
         max_nsend = nsend;
         printf("%d: max_nsend=%d\n",rank,max_nsend);
       }
       if (nrecv > max_nrecv) {
         max_nrecv = nrecv;
         printf("%d: max_nrecv=%d\n",rank,max_nrecv);
       }
       /*printf("%d: nsend=%d nrecv=%d\n",rank,nsend,nrecv);*/
#endif

        if (one_malloc) {
            memset(send_buf,0xff,MAXBUF*sizeof(DTYPE));
            memset(recv_buf,0xff,MAXBUF*sizeof(DTYPE));
        }
        else {
            send_buf = (DTYPE *) malloc(nsend*msg_size*sizeof(DTYPE));
            if (send_buf == NULL) {
              printf("%d: send_buf malloc() failed at iter %d with %d bytes\n",
                     rank,iter,nsend*msg_size*sizeof(DTYPE));
              MPI_Abort(MPI_COMM_WORLD,-1);
            }

            nrecv_size = nrecv*msg_size*sizeof(DTYPE);
            recv_buf = (DTYPE *) malloc(nrecv_size);
            if (recv_buf == NULL) {
              printf("%d: recv_buf malloc() failed at iter %d with %d bytes\n",
                     rank,iter,nrecv*msg_size*sizeof(DTYPE));
              MPI_Abort(MPI_COMM_WORLD,-1);
            }
        }

        /* init the recv buffers to a known value, we use this later 
         * to check for buffer errors
        */
       for (i = 0; i < nrecv*msg_size; i++)
          recv_buf[i] = -i;

        /********************************************************
         * This is the communications block. It can be setup to 
         * do expected MPI messages, unexpected MPI messages, or
         * random expected/unexpected MPI messages.
        ********************************************************/
        switch (mpi_behavior) {
            default:
            case 0:
                /* default, random expected/unexpected MPI messages */
                ioffset = 0;
                for (i = irecv = 0; i < nproc; i++)
                   if ((size = msg_size*the_matrix[i][rank]) > 0) {
                      ierror = MPI_Irecv(&recv_buf[ioffset], size, MPI_DTYPE, i, MPI_ANY_TAG,
                                         MPI_COMM_WORLD, &request[irecv]);
                      irecv++;
                      ioffset += size;
                   }

                for (i = m = 0; i < nproc; i++) {
                   ioffset = m;
                   for (j = size = 0; j < the_matrix[rank][i]; j++)
                      for (k = 0; k < msg_size; k++, m++, size++)
                         send_buf[m] = COOKIE + k + rank;
                   if (size > 0)
                      ierror = MPI_Send(&send_buf[ioffset], size, MPI_DTYPE, i, 0x4f000000 | iter,
                                        MPI_COMM_WORLD);
                }                
                break;

            case 1:
                /* expected MPI messages only */
                ioffset = 0;
                for (i = irecv = 0; i < nproc; i++)
                   if ((size = msg_size*the_matrix[i][rank]) > 0) {
                      ierror = MPI_Irecv(&recv_buf[ioffset], size, MPI_DTYPE, i, MPI_ANY_TAG,
                                         MPI_COMM_WORLD, &request[irecv]);
                      irecv++;
                      ioffset += size;
                   }

                 /* forces all messages to be expected */
                 ierror = MPI_Barrier(MPI_COMM_WORLD);

                for (i = m = 0; i < nproc; i++) {
                   ioffset = m;
                   for (j = size = 0; j < the_matrix[rank][i]; j++)
                      for (k = 0; k < msg_size; k++, m++, size++)
                         send_buf[m] = COOKIE + k + rank;
                   if (size > 0)
                      ierror = MPI_Send(&send_buf[ioffset], size, MPI_DTYPE, i, 0x4f000000 | iter,
                                        MPI_COMM_WORLD);
                }                
                break;

            case 2:
                /* unexpected MPI messages only */
                for (i = m = isend = 0; i < nproc; i++) {
                   ioffset = m;
                   for (j = size = 0; j < the_matrix[rank][i]; j++)
                      for (k = 0; k < msg_size; k++, m++, size++)
                         send_buf[m] = COOKIE + k + rank;
                   if (size > 0) {
                      ierror = MPI_Isend(&send_buf[ioffset], size, MPI_DTYPE, i, 0x4f000000 | iter,
                                        MPI_COMM_WORLD,&mpi_requests[isend]);
                      
                      isend++;
                   }
                }

                ioffset = 0;
                for (i = irecv = 0; i < nproc; i++)
                   if ((size = msg_size*the_matrix[i][rank]) > 0) {
                      ierror = MPI_Irecv(&recv_buf[ioffset], size, MPI_DTYPE, i, MPI_ANY_TAG,
                                         MPI_COMM_WORLD, &request[irecv]);
                      irecv++;
                      ioffset += size;
                   }
                break;

        }

        if (isend > 0) {
            ierror = MPI_Waitall(isend, mpi_requests, status);
        }
        
        if (irecv > 0) {
           ierror = MPI_Waitall(irecv, request, status);

           for (x = 0; x < irecv; x++) {
               MPI_Get_count(&status[x],MPI_BYTE,&recv_units);
               if (recv_units != sizeof(DTYPE)*msg_size*the_matrix[status[x].MPI_SOURCE][rank]) {
                  printf("proc %d: recved %d, expected %d\n",
                         rank,recv_units,sizeof(DTYPE)*msg_size*the_matrix[status[x].MPI_SOURCE][rank]);
               }
           }
        }
        /*********************************************************
         * End of the MPI communications block
        *********************************************************/


       m = 0;
       for (i = 0; i < nproc; i++)
          for (j = 0 ; j < the_matrix[i][rank]; j++)
             for (k = 0; k < msg_size; k++, m++)
                if (recv_buf[m] != (COOKIE + k + i)) {
                     x = m;
                     printf("err:%d <- %d,msg %d of %d,wrd %d of %d,%d(0x%0x) != %d(0x%0x),buf idx=%d,iter=%d,irecv=%d\n",
                            rank, i, j, the_matrix[i][rank]-1, k,msg_size-1,recv_buf[x],recv_buf[x],
                            (COOKIE + k + i),(COOKIE + k + i),x,iter,irecv);
                     if (verbose > 1) {
                         printf("err: buf[m-1]=%0x buf[m]=%0x buf[m+1]=%0x\n",
                                recv_buf[x-1],recv_buf[x],recv_buf[x+1]);
                         printf("err: &buf[m-1]=%p &buf[m]=%p &buf[m+1]=%p\n",
                                &recv_buf[x-1],&recv_buf[x],&recv_buf[x+1]);
                         printf("err: &buf_start=%p &buf_last=%p buf[last]=%0x\n",
                                &recv_buf[0],&recv_buf[nrecv*msg_size],recv_buf[nrecv*msg_size-1]);
                         temp_ulong = (unsigned long)&recv_buf[nrecv*msg_size];
                         temp_ulong = temp_ulong - (unsigned long)&recv_buf[0];
                         printf("err: buf size=%lu malloc size=%d nrecv=%d\n",
                                temp_ulong,nrecv_size,nrecv);
                     }
                     num_errors++;
                     /* while (1); */
                }     

       ierror = MPI_Barrier(MPI_COMM_WORLD);

        if (!one_malloc) {
            free(send_buf);
            free(recv_buf);
            send_buf = recv_buf = NULL;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    printf("%d: Exited MAIN LOOP.\n",rank);
    
    if (one_malloc) {
        free(send_buf_base);
        free(recv_buf_base);
    }

    ierror =  MPI_Finalize();
}


void handle_signals(int sig)
{
    switch (sig) {
        case SIGUSR1:
            /* printf("%d: Got SIGUSR1...\n",rank); */
            printf("%d: shutdown = %d\n",rank,shutdown);
            printf("%d: num_errors = %d\n",rank,num_errors);
            break;
        
        case SIGUSR2:
            printf("%d: Got SIGUSR2...\n",rank);
            shutdown = 1;
            break;
        
        case SIGTERM:
            shutdown = 1;
            break;
            
        default:
            1 == 1;  /* do nothing, avoid some compiler warning */
    }
    
    return;
}


DTYPE random_count()
{
    DTYPE i;
   
    if (msg_count == 0)
        return 1;
    else if (msg_count == 1)
        return 500;
    else if (msg_count == 2)
        return 701;
    else if (msg_count == 3) {
        i = rand() % RAND_MOD;
        return i;
    }
    else {
        i = rand() % RAND_MOD;
        if (i > 75) i = 0;
        return i;
    }
}


void usage(char *name)
{
    if (rank == 0) {
        printf("Usage: %s\n",name);
        printf("    -v verbose\n");
        printf("    -o one_malloc memory method\n");
        printf("    -m <number of words>  msg_size\n");
        printf("    -c <0,1,2,3> msg_count control\n");
        printf("       0 = tiny messages\n");
        printf("       1 = short messges, i.e. MPI short messages\n");
        printf("       2 = long messages, i.e. MPI long messages\n");
        printf("       3 = random message sizes\n");
        printf("    -a <alignment offset in bytes>\n");
        printf("    -b <0,1,2> MPI behavior\n");
        printf("       0 = random mix unexpected,expected MPI messages\n");
        printf("       1 = expected MPI messages only\n");
        printf("       2 = unexpected MPI messages only\n");
        
    }

    MPI_Finalize();
    exit(0);
}
