/*

    This work was produced at the University of California, Lawrence Livermore
    National Laboratory (UC LLNL) under contract no. W-7405-ENG-48 (Contract
    48) between the U.S. Department of Energy (DOE) and The Regents of the
    University of California (University) for the operation of UC LLNL. The
    rights of the Federal Government are reserved under Contract 48 subject to
    the restrictions agreed upon by the DOE and University as allowed under DOE
    Acquisition Letter 97-1.

    DISCLAIMER

    This work was prepared as an account of work sponsored by an agency of the
    United States Government. Neither the United States Government nor the
    University of California nor any of their employees, makes any warranty,
    express or implied, or assumes any liability or responsibility for the
    accuracy, completeness, or usefulness of any information, apparatus,
    product, or process disclosed, or represents that its use would not
    infringe privately-owned rights.  Reference herein to any specific
    commercial products, process, or service by trade name, trademark,
    manufacturer or otherwise does not necessarily constitute or imply its
    endorsement, recommendation, or favoring by the United States Government or
    the University of California. The views and opinions of authors expressed
    herein do not necessarily state or reflect those of the United States
    Government or the University of California, and shall not be used for
    advertising or product endorsement purposes.

    NOTIFICATION OF COMMERCIAL USE

    Commercialization of this product is prohibited without notifying the
    Department of Energy (DOE) or Lawrence Livermore National Laboratory
    (LLNL).

    UCRL-CODE-2001-028

    allred.c

    Syntax:
  
      allred [# of comp iterations] [inner iterations] [total samples]

      where comp iterations is the number of times that the computation
      loop is executed between MPI_Allreduce operations, inner iterations
      is the number of MPI_Allreduce operations between time samples, and
      total samples is the total number of time samples taken.
    
    Description:
      
      This application attempts to capture MPI_Allreduce performance in
      a simulated application setting in which compute phases alternate 
      with MPI_Allreduce operations.  The total compute time is
      subtracted from the compute+MPI_Allreduce time to calculate the
      MPI_Allreduce operation time average.

*/


#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include "util.h"


#define N         4

double crunch(double*, double*, double);
void   set(double*);
double doComp(int, double*, double*);
double getDoCompTime(int, double*, double*);
void   getLoopSamples(int, int, int, double**, double*, double*, int);
void   getOpSamples(int, int, int, double**, double*, double*, int);
void   printSamples(char*, double*, int);
void   doTest(int, int, int, int, int);

#define PRINT_SAMPLES

double crunch(double *a, double * b, double c)
{
     int i;

     for (i = 0; i < N; i++) c *= a[i] * b[i];
     
     return c;
}


void set(double *num)
{
     int i;

     srand((unsigned int) 0);

     for (i = 0; i < N; i++) num[i] = rand();
}


double doComp(int compIters, double* a, double* b)
{
     double c;
     int i;

     set(a);
     set(b);
     c = 0.0;

     for ( i = 0; i < compIters; i++ )
          c += crunch(a, b, c);

     return c;
}


void getMeanAndStddev(double* inSamples, int inSampleCount, 
                      double* mean, double* stddev)
{
     int i;
     double total, var;

     for ( i = 0, total = 0.0; i < inSampleCount; i++ )
     {
          total += inSamples[i];
     }

     *mean = total/(double)inSampleCount;

     for ( i = 0, var = 0.0; i < inSampleCount; i++ )
     {
          var += ((inSamples[i] - *mean)*(inSamples[i] - *mean));
     }
     
     var /= inSampleCount;

     *stddev = (double)sqrt(var);
}


double getDoCompTime(int compIters, double* a, double* b)
{
     double start;

     doComp(compIters, a, b);
     start = MPI_Wtime();
     doComp(compIters, a, b);

     return (MPI_Wtime() - start);
}


void getLoopSamples(int compIters, int innerIters, 
                    int testIters, double** samples, 
                    double* sendBuf, double* recvBuf, int messSize)
{
     double start, diff, compResult;
     int innerx, testx;

     *samples = (double*)malloc(testIters*sizeof(double));

     for ( testx = 0; testx < testIters; testx++ )
     {
          compResult = doComp(compIters, sendBuf, recvBuf);
          generic_barrier(MPI_COMM_WORLD);

          MPI_Allreduce(sendBuf, recvBuf, messSize, MPI_DOUBLE, 
                        MPI_SUM, MPI_COMM_WORLD);

          start = MPI_Wtime();

          for ( innerx = 0; innerx < innerIters; innerx++ )
          {
               compResult = doComp(compIters, sendBuf, recvBuf);
          }

          diff = MPI_Wtime() - start;

          MPI_Reduce(&diff, &((*samples)[testx]), 1, MPI_DOUBLE, 
                     MPI_MAX, 0, MPI_COMM_WORLD);
     }

     return;
}


void getOpSamples(int compIters, int innerIters, 
                  int testIters, double** samples, 
                  double* sendBuf, double* recvBuf, int messSize)
{
     double start, diff, compResult;
     int innerx, testx;

     *samples = (double*)malloc(testIters*sizeof(double));

     for ( testx = 0; testx < testIters; testx++ )
     {
          compResult = doComp(compIters, sendBuf, recvBuf);

          generic_barrier(MPI_COMM_WORLD);
          MPI_Allreduce(sendBuf, recvBuf, messSize, MPI_DOUBLE, 
                        MPI_SUM, MPI_COMM_WORLD);

          start = MPI_Wtime();

          for ( innerx = 0; innerx < innerIters; innerx++ )
          {
               MPI_Allreduce(sendBuf, recvBuf, messSize, MPI_DOUBLE, 
                             MPI_SUM, MPI_COMM_WORLD);

               compResult = doComp(compIters, sendBuf, recvBuf);
          }

          diff = MPI_Wtime() - start;

          MPI_Reduce(&diff, &((*samples)[testx]), 1, MPI_DOUBLE, 
                     MPI_MAX, 0, MPI_COMM_WORLD);
     }

     return;
}


void printSamples(char* label, double* samples, int sampleCount)
{
     int i;
     double mean, stddev;

     printf("\n%s\n\n", label);

     for ( i = 0; i <  sampleCount; i++ )
          printf("  samples[%2d] : %19.10f\n", i, samples[i]);

     printf("\n");

     getMeanAndStddev(samples, sampleCount, &mean, &stddev);
     printf("  mean        : %f\n", mean);
     printf("  stddev      : %f\n", stddev);
     printf("  stddev/mean : %f\n", stddev/mean);
}


void doTest(int messSize, int reportRank, int compIters, 
            int innerIters, int testIters)
{
     int x, totalSamples, myRank, worldSize;
     double *loopSamples, *opSamples;
     double totalLoopTime, totalOpTime;
     double loopMean, opMean;
     double *sendBuf, *recvBuf;
     double wtick;

     MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
     MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

     if ( (sendBuf = (double*)malloc(messSize*sizeof(double))) == NULL )
     {
          printf("Failed to malloc sendBuf\n");
          exit(-1);
     }

     if ( (recvBuf = (double*)malloc(messSize*sizeof(double))) == NULL )
     {
          printf("Failed to malloc recvBuf\n");
          exit(-1);
     }

     /*  Collect Computation Loop Timing Data  */
     getLoopSamples(compIters, innerIters, testIters, &loopSamples, 
                    sendBuf, recvBuf, messSize);

     /*  Collect Operation & Computation Loop Timing Data  */
     getOpSamples(compIters, innerIters, testIters, &opSamples, 
                  sendBuf, recvBuf, messSize);

     totalSamples = testIters;

     totalLoopTime = 0.0;
     totalOpTime   = 0.0;
     wtick         = MPI_Wtick();

     listRankLocations(myRank, worldSize);

     if ( myRank == reportRank )
     {
          for ( x = 0; x < totalSamples; x++ )
          {
               totalLoopTime += loopSamples[x];
               totalOpTime   += opSamples[x];
          }

          loopMean = totalLoopTime / totalSamples;
          opMean   = totalOpTime / totalSamples;

#ifdef PRINT_SAMPLES
          printSamples("Loop Base Samples", loopSamples, testIters);
          printSamples("Loop Op Samples", opSamples, testIters);
#endif
          printf("\nMPI Allreduce test\n");
          printf("  for %d MPI processes, message size %d bytes, \n", 
                 worldSize, messSize*sizeof(double));
          printf("  %d operations per sample, %d samples\n\n", 
                 innerIters, totalSamples);
          printf("  Wtick resolution           : %11.3f us\n", wtick*1000000);
          printf("  Time between Allreduce     : %11.3f us\n\n", 
                 (getDoCompTime(compIters, sendBuf, recvBuf)*1000000));
          printf("  Mean Compute Loop Time     : %11.3f us\n", loopMean*1000000);
          printf("  Ticks per Compute Loop     : %11.0f\n", loopMean/wtick);
          printf("  Mean Op Loop Time          : %11.3f us\n", opMean*1000000);
          printf("  Ticks per Op Loop          : %11.0f\n\n", opMean/wtick);
          printf("  Op mean                    : %11.3f us\n\n", 
                 ((totalOpTime - totalLoopTime)/(totalSamples*innerIters))*1000000);
     }

     free(loopSamples);
     free(opSamples);
     free(sendBuf);
     free(recvBuf);
}


int main(int argc, char** argv)
{
     int messSize, compIters, innerIters, testIters, reportRank;

     MPI_Init(&argc, &argv);

     if ( argc != 4 )
     {
          int rank;

          MPI_Comm_rank(MPI_COMM_WORLD, &rank);

          if ( rank == 0 )
               printf("\n  %s [# of comp iterations] [inner iterations] [total samples]\n\n", argv[0]);
          MPI_Finalize();
          exit(-1);
     }

     messSize    = 4;             /* number of doubles in Allreduce message */
     reportRank  = 0;             /* Rank printing output                   */
     compIters   = atoi(argv[1]); /* Number of computation iterations       */
     innerIters  = atoi(argv[2]); /* Number of inner loop iterations        */
     testIters   = atoi(argv[3]); /* Number of total sample iterations      */

     doTest(messSize, reportRank, compIters, innerIters, testIters);

     MPI_Finalize();

     exit(0);
}
