#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include "mpi.h"

#ifdef USE_LONG
#define DTYPE       long
#define MPI_DTYPE   MPI_LONG
#else
#define DTYPE       int
#define MPI_DTYPE   MPI_INT
#endif

#define RAND_MOD    100
#define MAXBUF      (nproc*RAND_MOD*MSG_SIZE)
#define MSG_SIZE    3
#define COOKIE      8301965
/*#define COOKIE      0x0fff0000*/


extern void dump_mpi_debug();
extern void *my_memcpy(void *dest, const void *src, size_t n);

DTYPE random_count()
{
   DTYPE i;
   
#ifdef SHORT
   return 667;
#elif REALLY_SHORT
   return 1;
#elif LONG
   return 701;
#else
   i = rand()%RAND_MOD;
   if (i > 75) i = 0;
   return i;
#endif
}

main(int argc, char **argv)
{
   /*int MSG_SIZE = 9, COOKIE = 8301965;*/
   DTYPE **the_matrix, *send_buf, *recv_buf, *rcv;
   DTYPE ***all_matrices = NULL;
   DTYPE *send_matrix = NULL;

   int i, j, k, m, rank, nproc, nsend, nrecv, nneigh, irecv, isend, size,
       ioffset, ierror, iter, count;
   MPI_Request *request;
   MPI_Status *status;
   int max_nsend = 0;
   int max_nrecv = 0;
   int x,y,master,slave;
   MPI_Status mpi_status;
   int recv_units;
   unsigned long temp_ulong;
   int nrecv_size;

   /*mallopt(M_MMAP_MAX,0);*/
   
   ierror = MPI_Init(&argc,&argv);

   ierror = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   ierror = MPI_Comm_size(MPI_COMM_WORLD, &nproc);

   the_matrix = (DTYPE **) malloc(nproc*sizeof(DTYPE *));
   for (i = 0; i < nproc; i++)
      the_matrix[i] = (DTYPE *) malloc(nproc*sizeof(DTYPE));
   request = (MPI_Request *) malloc(nproc*sizeof(MPI_Request));
   status = (MPI_Status *) malloc(nproc*sizeof(MPI_Status));
   rcv = (DTYPE *) malloc(nproc*sizeof(DTYPE));

#ifdef ONE_MALLOC
      send_buf = (DTYPE *) malloc(MAXBUF*sizeof(DTYPE));
      nrecv_size = MAXBUF*sizeof(DTYPE);
      recv_buf = (DTYPE *) malloc(nrecv_size);
      if (1) {
        printf("%d send_buf: %d messages w/ %d bytes @ 0x%08x\n",rank,nproc*RAND_MOD,
               MAXBUF*sizeof(DTYPE),send_buf);
        printf("%d recv_buf: %d messages w/ %d bytes @ 0x%08x\n",rank,nproc*RAND_MOD,
               MAXBUF*sizeof(DTYPE),recv_buf);
      }
#endif

   iter = 0;
   while (1) {

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

#ifdef MATRIX_CHECK
    if (rank == 0) {
        if (all_matrices == NULL) {
            all_matrices = (DTYPE***)malloc(nproc*sizeof(DTYPE*));
            for (i = 0; i < nproc; i++) {
                all_matrices[i] = (DTYPE**)malloc(nproc*sizeof(DTYPE*));
                for (j = 0; j < nproc; j++)
                    all_matrices[i][j] = (DTYPE*)malloc(nproc*sizeof(DTYPE*));
            }
        }

        for (i = 0; i < nproc; i++) {
            memcpy(all_matrices[0][i],the_matrix[i],nproc*sizeof(DTYPE));
        }
        
        for (i = 1; i < nproc; i++) {
            for (j = 0; j < nproc; j++) {
                MPI_Recv(all_matrices[i][j],nproc,MPI_DTYPE,i,j,MPI_COMM_WORLD,&mpi_status);
            }
        }
    }
    else {
        if (send_matrix == NULL) {
            send_matrix = (DTYPE*)malloc(nproc*nproc*sizeof(DTYPE));
            if (send_matrix == NULL) {
                printf("error malloc()ing send_matrix\n");
                MPI_Abort(MPI_COMM_WORLD,-3);
            }
        }
        
        for (i = 0; i < nproc; i++) {
            memcpy(&send_matrix[i*nproc],the_matrix[i],nproc*sizeof(DTYPE));
        }
        
        for (i = 0; i < nproc; i++) {
            MPI_Send(the_matrix[i],nproc,MPI_DTYPE,0,i,MPI_COMM_WORLD);
        }
    }
    
    if (rank == 0) {
        for (master = 0; master < nproc; master++) {
            for (slave = 0; slave < nproc; slave++)
                for (x = 0; x < nproc; x++)
                    for (y = 0; y < nproc; y++) {
                        if (all_matrices[master][x][y] != all_matrices[slave][x][y]) {
                            printf("proc %d matrix[%d][%d] is %ld BUT proc %d matrix[%d][%d] is %ld\n",
                                   master,x,y,all_matrices[master][x][y],slave,x,y,all_matrices[slave][x][y]);
                        }
                    }
        }
    }
#endif

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

#ifdef ONE_MALLOC
      memset(send_buf,0xff,MAXBUF*sizeof(DTYPE));
      memset(recv_buf,0xff,MAXBUF*sizeof(DTYPE));
#else      
      send_buf = (DTYPE *) malloc(nsend*MSG_SIZE*sizeof(DTYPE));
      if (send_buf == NULL) {
        printf("%d: send_buf malloc() failed at iter %d with %d bytes\n",
               rank,iter,nsend*MSG_SIZE*sizeof(DTYPE));
        MPI_Abort(MPI_COMM_WORLD,-1);
      }
      
      nrecv_size = nrecv*MSG_SIZE*sizeof(DTYPE);
      recv_buf = (DTYPE *) malloc(nrecv_size);
      if (recv_buf == NULL) {
        printf("%d: recv_buf malloc() failed at iter %d with %d bytes\n",
               rank,iter,nrecv*MSG_SIZE*sizeof(DTYPE));
        MPI_Abort(MPI_COMM_WORLD,-1);
      }
#endif
      for (i = 0; i < nrecv*MSG_SIZE; i++)
         recv_buf[i] = -i;

      ioffset = 0;
      for (i = irecv = 0; i < nproc; i++)
         if ((size = MSG_SIZE*the_matrix[i][rank]) > 0) {
            ierror = MPI_Irecv(&recv_buf[ioffset], size, MPI_DTYPE, i, MPI_ANY_TAG,
                               MPI_COMM_WORLD, &request[irecv]);
            irecv++;
            ioffset += size;
         }
      
      /*ierror = MPI_Barrier(MPI_COMM_WORLD);*/
         
      for (i = m = 0; i < nproc; i++) {
         ioffset = m;
         for (j = size = 0; j < the_matrix[rank][i]; j++)
            for (k = 0; k < MSG_SIZE; k++, m++, size++)
               send_buf[m] = COOKIE + k + rank;
         if (size > 0)
            ierror = MPI_Send(&send_buf[ioffset], size, MPI_DTYPE, i, 0x4f000000 | iter,
                              MPI_COMM_WORLD);
      }

      if (irecv > 0) {
         ierror = MPI_Waitall(irecv, request, status);

         for (x = 0; x < irecv; x++) {
             MPI_Get_count(&status[x],MPI_BYTE,&recv_units);
             if (recv_units != sizeof(DTYPE)*MSG_SIZE*the_matrix[status[x].MPI_SOURCE][rank]) {
                printf("proc %d: recved %d, expected %d\n",
                       rank,recv_units,sizeof(DTYPE)*MSG_SIZE*the_matrix[status[x].MPI_SOURCE][rank]);
             }
         }
      }

      m = 0;
      for (i = 0; i < nproc; i++)
         for (j = 0 ; j < the_matrix[i][rank]; j++)
            for (k = 0; k < MSG_SIZE; k++, m++)
               if (recv_buf[m] != (COOKIE + k + i)) {
                    x = m;
                    printf("err:%d <- %d,msg %d of %d,wrd %d of %d,%d(0x%0x) != %d(0x%0x),buf idx=%d,iter=%d,irecv=%d\n",
                           rank, i, j, the_matrix[i][rank]-1, k,MSG_SIZE-1,recv_buf[x],recv_buf[x],
                           (COOKIE + k + i),(COOKIE + k + i),x,iter,irecv);
                    printf("err: buf[m-1]=%0x buf[m]=%0x buf[m+1]=%0x\n",
                           recv_buf[x-1],recv_buf[x],recv_buf[x+1]);
                    printf("err: &buf[m-1]=%p &buf[m]=%p &buf[m+1]=%p\n",
                           &recv_buf[x-1],&recv_buf[x],&recv_buf[x+1]);
                    printf("err: &buf_start=%p &buf_last=%p buf[last]=%0x\n",
                           &recv_buf[0],&recv_buf[nrecv*MSG_SIZE],recv_buf[nrecv*MSG_SIZE-1]);
                    temp_ulong = (unsigned long)&recv_buf[nrecv*MSG_SIZE];
                    temp_ulong = temp_ulong - (unsigned long)&recv_buf[0];
                    printf("err: buf size=%lu malloc size=%d nrecv=%d\n",
                           temp_ulong,nrecv_size,nrecv);
                    
                    /* while (1); */
               }     

      ierror = MPI_Barrier(MPI_COMM_WORLD);

#ifndef ONE_MALLOC
      free(send_buf);
      free(recv_buf);
      send_buf = recv_buf = NULL;
#endif
   }

#ifdef ONE_MALLOC
      free(send_buf);
      free(recv_buf);
      send_buf = recv_buf = NULL;
#endif

   ierror =  MPI_Finalize();
}
