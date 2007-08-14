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

    rma.c

    Description:

    This test uses MPI_Put and MPI_Get to determine unidirectional and
    bidirectional RMA bandwidth and average epoch length for a specified 
    number of operations per epoch.

    The bandwidth is calculated by dividing the total size of all RMA
    operations sizes by the time sample.

*/


#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <mpi.h>
#include "util.h"


char* outputFormat     = "  %-12s   %9d   %12d   %10d  %15.3f   %12.3f\n";
char* outputCharFormat = "  %-12s   %9s   %12s   %10s  %15s   %12s\n";

typedef enum { Uniput, Uniget, Biput, Biget } testType;
char *testLabels[4] = 
   { "Unidir. Put", "Unidir. Get", "Bidir. Put", "Bidir. Get" };

static char* command;

double runTestForProcessSet(testType testID, int procs, 
                            int messStart, int messStop, int messFactor, 
                            int epochOps, int epochIters, int rank, int wsize,
                            char allocPattern, int useBarrier, 
                            int useNearestRank, int procsPerNode);

double runTest(testType test, int epochOps, int epochIters, int bufsize, 
               char* winbuf, MPI_Win win, int rank, int currtarg, 
               int wsize, int useBarrier, MPI_Comm* activeComm);

void printReportHeader(void);
void printUse(void);


main(int argc, char** argv)
{
  int rank, wsize, epochOps, epochIters, procs, i;
  int messStart, messStop, messFactor, procsPerNode;
  int printPairs, useBarrier, useNearestRank, procIdx, *procList, procListSize;
  char *procFile, allocPattern;
  double minTime  = 99999.0, timeResult;
  command = argv[0];

  MPI_Init(&argc, &argv);

  MPI_Comm_size(MPI_COMM_WORLD, &wsize);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if ( !processArgs(argc, argv, rank, wsize, &epochOps, &epochIters, 
                    &messStart, &messStop, &messFactor, 
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

  /*  
      Main test loop iterates through tests, concurrent communicating 
      process pairs, and message sizes.
  */
  for ( i = 0; i < 4; i++ ) 
  {
    for ( procIdx = 0; procIdx < procListSize; procIdx++ )
    {
      procs = procList[procIdx];

      if ( rank == 0 && printPairs )
      {
        printActivePairs(procs, rank, wsize, procsPerNode, 
                         allocPattern, useNearestRank);
      }

      timeResult = runTestForProcessSet((testType)i, procs, messStart,  
                                        messStop, messFactor, epochOps, 
                                        epochIters, rank, wsize,
                                        allocPattern, useBarrier,
                                        useNearestRank, procsPerNode);
      if ( timeResult < minTime )
        minTime = timeResult;
    }
  }
  
  printReportFooter(minTime, rank, wsize, procsPerNode, useNearestRank);

  MPI_Finalize();

  exit(0);
}


double runTestForProcessSet(testType testID, int procs, 
                            int messStart, int messStop, int messFactor, 
                            int epochOps, int epochIters, int rank, int wsize,
                            char allocPattern, int useBarrier, 
                            int useNearestRank, int procsPerNode)
{
  char* winbuf, sendBuf[1];
  MPI_Win win;
  MPI_Status stat;
  MPI_Group worldGroup, activeGroup, pairGroup;
  MPI_Comm activeComm, pairComm;
  int *activeIds, pairIds[2];
  double sampleData, sampleMax = 0;
  int currtarg, i;
  int bufsize, totbufsize;
  int newTarg, limit;
  int allocRet;

  limit = procs/2;

  for ( i = messStart; i <= messStop; 
        i *= messFactor )
  {
    totbufsize = i;
  }

  if ( testID == Biput || testID == Biget )
    totbufsize *= 2;

  currtarg = getTargetRank(rank, wsize, procsPerNode, useNearestRank);
  sampleData = 0;

  MPI_Comm_group(MPI_COMM_WORLD, &worldGroup);
  
  /*  Create Communicator of all active processes  */
  activeIds = (int*)malloc(procs*sizeof(int));
  for ( i = 0; i < limit; i++ )
  {
    getPair(i, wsize, procsPerNode, allocPattern, useNearestRank,
            &(activeIds[i]), &(activeIds[limit+i]));
  }

  MPI_Group_incl(worldGroup, procs, activeIds, &activeGroup);
  MPI_Comm_create(MPI_COMM_WORLD, activeGroup, &activeComm);
  free(activeIds);

  /*  Create Communicator for active process pair  */
  pairIds[0] = rank < currtarg ? rank : currtarg;
  pairIds[1] = rank < currtarg ? currtarg : rank;

  MPI_Group_incl(worldGroup, 2, pairIds, &pairGroup);
  MPI_Comm_create(MPI_COMM_WORLD, pairGroup, &pairComm);

  newTarg = rank < currtarg ? 1 : 0;

  MPI_Group_free(&worldGroup);
  MPI_Group_free(&activeGroup);
  MPI_Group_free(&pairGroup);

  if ( isActiveProc(rank, wsize, procsPerNode, procs, 
                    allocPattern, useNearestRank) ) 
  {
    /*  Create RMA window for process pair  */
    MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
    allocRet = MPI_Alloc_mem(totbufsize*epochOps, MPI_INFO_NULL, &winbuf);

    if ( allocRet != MPI_SUCCESS )
    {
      char errMess[MPI_MAX_ERROR_STRING];
      int messLen;

      MPI_Error_string(allocRet, errMess, &messLen);

      printf("\nFailed to allocate enough memory for window.\n");
      printf("Use fewer operations per epoch or smaller message sizes.\n\n");

      printf("%s\n", errMess);
      MPI_Abort(MPI_COMM_WORLD, allocRet);
    }

    MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_ARE_FATAL);
     
    MPI_Win_create(winbuf, totbufsize*epochOps, 1, MPI_INFO_NULL, 
                   pairComm, &win);

    memset(winbuf, 0, totbufsize*epochOps);

    /*  Ensure communication link has been established  */
    if ( rank < currtarg )
      MPI_Recv(sendBuf, 1, MPI_CHAR, currtarg, 0, MPI_COMM_WORLD, &stat);
    else
      MPI_Send(sendBuf, 1, MPI_CHAR, currtarg, 0, MPI_COMM_WORLD);

    for ( bufsize = messStart; bufsize <= messStop; bufsize *= messFactor )
    {
      /*  Measured Loop  */
      sampleData = runTest(testID, epochOps, epochIters, bufsize, winbuf, win,
                           rank, newTarg, wsize, useBarrier, &activeComm);

      /* Get longest epoch length from all tasks */
      MPI_Reduce(&sampleData, &sampleMax, 1, MPI_DOUBLE, MPI_MAX, 
                 0, activeComm);

      if ( rank == 0 )
      {
        if ( testID == Uniput || testID == Uniget )
        {
          printf(outputFormat, testLabels[testID], procs, bufsize, epochOps,
                 (((double)bufsize*epochOps*epochIters*(procs/2))/sampleMax)/1000000,
                 (sampleMax/(epochOps*epochIters))*1000000);
        }
        else
        {
          printf(outputFormat, testLabels[testID], procs, bufsize, epochOps,
                 (((double)bufsize*epochOps*epochIters*procs)/sampleMax)/1000000,
                 (sampleMax/(epochOps*epochIters))*1000000);
        }
      }
    }

    if ( rank == 0 )
      putchar('\n');

    /*  Free MPI objects  */
    MPI_Win_free(&win);
    MPI_Free_mem(winbuf);
    MPI_Comm_free(&activeComm);
    MPI_Comm_free(&pairComm);
  }

  generic_barrier(MPI_COMM_WORLD);
  
  return sampleMax;
}


double runTest(testType test, int epochOps, int epochIters, int bufsize, 
               char* winbuf, MPI_Win win, int rank, int currtarg,
               int wsize, int useBarrier, MPI_Comm* activeComm)
{
  double start, sampleData;
  int i, j;
  char* buffer;
  int locbufsize;

  locbufsize = epochOps * bufsize * sizeof(char);
  
  buffer = (char*)malloc(locbufsize);

  if ( buffer == NULL )
  {
    fprintf(stderr, "Failed to allocate RMA buffer!\n");
    MPI_Abort(MPI_COMM_WORLD, -1);
  }

  memset(buffer, 0, locbufsize);
  
  /*  Start first epoch  */
  MPI_Win_fence(0, win);
  
  generic_barrier(*activeComm);
  start = MPI_Wtime();

  switch (test)
  {
  case Uniput:
    for ( j = 0; j < epochIters; j++ )
    {
      if ( rank < wsize/2 )
      {
        for ( i = 0; i < epochOps; i++ )
        {
          MPI_Put(buffer+(i*bufsize), bufsize, MPI_CHAR, 
                  currtarg, (i*bufsize), bufsize, MPI_CHAR, win);
        }
      }
  
      MPI_Win_fence(0, win);
    }
    break;
  case Uniget:
    for ( j = 0; j < epochIters; j++ )
    {
      if ( rank < wsize/2 )
      {
        for ( i = 0; i < epochOps; i++ )
        {
          MPI_Get(buffer+(i*bufsize), bufsize, MPI_CHAR, 
                  currtarg, (i*bufsize), bufsize, MPI_CHAR, win);
        }
      }
  
      MPI_Win_fence(0, win);
    }
    break;
  case Biput:
    for ( j = 0; j < epochIters; j++ )
    {
      for ( i = 0; i < epochOps; i++ )
      {
        MPI_Put(buffer+(bufsize*i), bufsize, MPI_CHAR, 
                currtarg, bufsize*i, bufsize, MPI_CHAR, win);
      }
  
      MPI_Win_fence(0, win);
    }
    break;
  case Biget:
    for ( j = 0; j < epochIters; j++ )
    {
      for ( i = 0; i < epochOps; i++ )
      {
        MPI_Get(buffer+(bufsize*i), bufsize, MPI_CHAR, 
                currtarg, bufsize*i, bufsize, MPI_CHAR, win);
      }
  
      MPI_Win_fence(0, win);
    }
    break;
  }

  if ( useBarrier )
    generic_barrier(*activeComm);

  sampleData = MPI_Wtime() - start;

  free(buffer);

  return sampleData;
}


void printReportHeader(void)
{
  char buf[128];
  size_t i, buflen;

  printf("\nUnidirectional and Bidirectional Communication Test\n\n");
  sprintf(buf, outputCharFormat, "Test", "Processes", "Message Size", 
          "Ops/Epoch", "Bandwidth (MB)", "Op time (us)");
  printf("%s", buf);

  buflen = strlen(buf);

  memset(buf, 0, 128);

  for ( i = 0; i < buflen; i++ )
    buf[i] = '-';

  printf("%s\n", buf);
}


void printUse(void)
{
  printf("\n%s : MPI interprocess communication benchmark for one-sided operations\n\n", command);
  printf("  syntax: %s [OPTION]...\n\n", command);
  printf("    -b [message start size]                        default=%d\n", MESS_START_DEF);
  printf("    -c [number of epochs between measurements]\n");
  printf("    -e [message stop  size]                        default=%d\n", MESS_STOP_DEF);
  printf("    -f [process count source file]\n");
  printf("    -o [number of operations per epoch]\n");
  printf("    -p [allocate processes: c(yclic) or b(lock)]   default=%c\n", ALLOC_DEF);
  printf("    -s [message size increase factor]              default=%d\n",MESS_FACTOR_DEF);
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

