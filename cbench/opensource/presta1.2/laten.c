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


    laten.c

    Description:

      This test uses MPI_Send and MPI_Recv to perform 0-length ping-pong 
      messages between pairs of MPI processes.  The latency for a single
      message is calculated by dividing the round-trip time by 2.
      
*/


#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <mpi.h>
#include "util.h"


static char* outputFormat     = "  %20d   %17.3f\n";
static char* outputCharFormat = "  %20s   %17s\n";
static char* command;

void printReportHeader(void);
void printUse(void);
void printParameters(int iters, char *procFile, int procsPerNode, 
                     char allocPattern, int useBarrier);


main(int argc, char** argv)
{
  int rank, wsize, iters, i, procs, currtarg, dummy;
  double diff = 0.0;
  double start, max, mintime = 9999;
  MPI_Status stat;
  char comBuf;
  MPI_Comm activeComm;
  char*  procFile       = NULL;
  int*   procList       = NULL;
  int    procListSize;
  int    messStart, messStop, messFactor;
  int    procsPerNode, procIdx, useBarrier, printPairs, useNearestRank;
  char   allocPattern;

  command = argv[0];

  MPI_Init(&argc, &argv);

  MPI_Comm_size(MPI_COMM_WORLD, &wsize);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if ( !processArgs(argc, argv, rank, wsize, &iters, 
                    &dummy, &messStart, &messStop, &messFactor, 
                    &procFile, &procsPerNode, &allocPattern, 
                    &printPairs, &useBarrier, &useNearestRank) )
  {
    if ( rank == 0 )
      printUse();

    MPI_Finalize();
    exit(-1);
  }

  if ( ! getProcList(procFile, wsize, &procList, &procListSize,
                     procsPerNode, allocPattern) )
  {
    if ( procFile )
      printf("Failed to get process list from file %s.\n", procFile);
    else
      printf("Failed to allocate process list.\n");

    exit(-1);
  }

  if ( rank == 0 )
    printReportHeader();

  currtarg = getTargetRank(rank, wsize, procsPerNode, useNearestRank);

  for ( procIdx = 0; procIdx < procListSize; procIdx++ )
  {
    procs = procList[procIdx];

    if ( printPairs )
    {
      printActivePairs(procs, rank, wsize, procsPerNode, 
                       allocPattern, useNearestRank);
    }

    /*  Create Communicator of all active processes  */
    createActiveComm(procs, rank, wsize, procsPerNode, 
                     allocPattern, printPairs, useNearestRank, &activeComm);
    

    if ( isActiveProc(rank, wsize, procsPerNode, procs, 
                      allocPattern, useNearestRank) )
    {
      if ( rank < currtarg )
      {
        /*  Ensure pair communication has been initialized  */
        MPI_Recv(&comBuf, 0, MPI_INT, currtarg, 0, MPI_COMM_WORLD, &stat);
        MPI_Send(&comBuf, 0, MPI_INT, currtarg, 0, MPI_COMM_WORLD);
      }
      else 
      {
        /*  Ensure pair communication has been initialized  */
        MPI_Send(&comBuf, 0, MPI_INT, currtarg, 0, MPI_COMM_WORLD);
        MPI_Recv(&comBuf, 0, MPI_INT, currtarg, 0, MPI_COMM_WORLD, &stat);
      }
    
      generic_barrier(activeComm);
      generic_barrier(activeComm);

      if ( rank < currtarg )
      {
        /*  Time operation loop  */
        start = MPI_Wtime();

        for ( i = 0; i < iters; i++ )
        {
          MPI_Send(&comBuf, 0, MPI_INT, currtarg, 0, MPI_COMM_WORLD);
          MPI_Recv(&comBuf, 0, MPI_INT, currtarg, 0, MPI_COMM_WORLD, &stat);
        }
      }
      else 
      {
        /*  Time operation loop  */
        start = MPI_Wtime();

        for ( i = 0; i < iters; i++ )
        {
          MPI_Recv(&comBuf, 0, MPI_INT, currtarg, 0, MPI_COMM_WORLD, &stat);
          MPI_Send(&comBuf, 0, MPI_INT, currtarg, 0, MPI_COMM_WORLD);
        }
      }

      if ( useBarrier )
        generic_barrier(activeComm);

      diff = MPI_Wtime() - start;
    }

    if ( activeComm != MPI_COMM_NULL )
      MPI_Comm_free(&activeComm);

    /*  Get maximum sample length  */
    MPI_Reduce(&diff, &max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if ( rank == 0 )
    {
      if ( max < mintime )
        mintime = max;

      printf(outputFormat, procs, max/iters/2*1000000);
    }
  }
  
  if ( rank == 0 )
  {
    printParameters(iters, procFile, procsPerNode, 
                    allocPattern, useBarrier);
  }

  printReportFooter(mintime, rank, wsize, procsPerNode, useNearestRank);

  MPI_Finalize();

  exit(0);
}


void printReportHeader(void)
{
  char buf[128];
  size_t i, len;

  printf("MPI Bidirectional latency test (Send/Recv)\n");
  sprintf(buf, outputCharFormat, "Processes", "Max Latency (us)");
  printf("%s", buf);

  len = strlen(buf);

  for ( i = 1; i < len; i++ )
    putchar('-');

  putchar('\n');
}


void printUse(void)
{
  printf("\n%s : MPI interprocess communication latency benchmark\n\n", command);
  printf("  syntax: %s [OPTION]...\n\n", command);
  printf("    -f [process count source file]\n");
  printf("    -o [number of operations between measurements]\n");
  printf("    -p [allocate processes: c(yclic) or b(lock)]   default=%c\n", ALLOC_DEF);
  printf("    -t [processes per SMP]\n");
  printf("    -h : print use information\n");
  printf("    -i : print process pair information            default=%s\n",
        PRINT_PAIRS_DEF ? "true" : "false" );
  printf("    -n : do not use barrier within measurement     default=%s\n",
        USE_BARRIER_DEF ? "barrier used" : "barrier not used");
  printf("    -r : partner processes with nearby rank        default=%s\n",
        NEAREST_RANK_DEF ? "true" : "false" );
  printf("\n");
}


void printParameters(int iters, char *procFile, int procsPerNode, 
                     char allocPattern, int useBarrier)
{
  printf("\nTest Parameters\n");
  printf("---------------\n");
  printf("Operations per measurement : %8d\n", iters);

  if ( procFile )
    printf("Process count file : %s\n", procFile);

  if (procsPerNode > 0 )
    printf("Processes per SMP          : %8d\n", procsPerNode);

  if ( allocPattern == 'b' )
    printf("Process pair allocation    : %8s\n", "block");
  else
    printf("Process pair allocation    : %8s\n", "cyclic");

  if ( ! useBarrier )
    printf("\nBarrier not included in measurement.\n");
  
  printf("\n");
}
