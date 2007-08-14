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


    util.c

    Provides functions common to benchmarks.

*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <mpi.h>
#include "util.h"

#define GENERIC_BARRIER_TAG 1


/*
    If the Wtick granularity for Wtime is coarser than the
    overhead of calling MPI_Wtime, then approximate the MPI_Wtime
    overhead by counting calls within one tick.
*/

double getWtimeOh()
{
  double start, stop, diff;
  int count;

  count = 1;
  start = MPI_Wtime();
  stop  = MPI_Wtime();
  diff  = stop - start;

  if ( diff == 0 || MPI_Wtime() == stop )
  {
    stop = MPI_Wtime();
    while ( (start = MPI_Wtime()) == stop );
    while ( (stop = MPI_Wtime()) == start )
      count++;
    return (stop - start)/count;
  }

  return stop - start;
}


/* This barrier implementation is taken from the Sphinx MPI benchmarks
   and is based directly on the MPICH barrier implementation...        */

void generic_barrier (MPI_Comm curcomm)
{
  int        rank, size, N2_prev, surfeit;
  int        d, dst, src, temp;
  MPI_Status status;

  /* Intialize communicator size */
  (void) MPI_Comm_size ( curcomm, &size );

  /* If there's only one member, this is trivial */
  if ( size > 1 ) {

    MPI_Comm_rank ( curcomm, &rank );

    for (temp = size, N2_prev = 1; 
	 temp > 1; temp >>= 1, N2_prev <<= 1) /* NULL */;

    surfeit = size - N2_prev;

    /* Perform a combine-like operation */
    if ( rank < N2_prev ) {
      if( rank < surfeit ) {

        /* get the fanin letter from the upper "half" process: */
        dst = N2_prev + rank;

        MPI_Recv((void *)0, 0, MPI_INT, dst,
		 GENERIC_BARRIER_TAG, curcomm, &status);
      }

      /* combine on embedded N2_prev power-of-two processes */
      for (d = 1; d < N2_prev; d <<= 1) {
        dst = (rank ^ d);

        MPI_Sendrecv( (void *)0,0,MPI_INT,dst, GENERIC_BARRIER_TAG,
                     (void *)0,0,MPI_INT,dst, GENERIC_BARRIER_TAG, 
                     curcomm, &status);
      }

      /* fanout data to nodes above N2_prev... */
      if ( rank < surfeit ) {
        dst = N2_prev + rank;
        MPI_Send( (void *)0, 0, MPI_INT, dst, GENERIC_BARRIER_TAG, curcomm);
      }
    } 
    else {
      /* fanin data to power of 2 subset */
      src = rank - N2_prev;
      MPI_Sendrecv( (void *)0, 0, MPI_INT, src, GENERIC_BARRIER_TAG,
                   (void *)0, 0, MPI_INT, src, GENERIC_BARRIER_TAG, 
                   curcomm, &status);
    }
  } 
  return; 
}


void printCommTargets(int rank, int wsize, 
                      int procsPerNode, int useNearestRank)
{
  char *procNames;
  char myProcName[MPI_MAX_PROCESSOR_NAME];
  int nameSize, currtarg, i;

  if ( rank == 0 )
    procNames = (char*)malloc(wsize*MPI_MAX_PROCESSOR_NAME*sizeof(char));

  MPI_Get_processor_name(myProcName, &nameSize);

  MPI_Gather(myProcName, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, procNames, 
             MPI_MAX_PROCESSOR_NAME, MPI_CHAR, 0, MPI_COMM_WORLD);

  if ( rank == 0 )
  {
    putchar('\n');

    for ( i = 0; i < wsize; i++ )
    {
      currtarg = getTargetRank(i, wsize, procsPerNode, useNearestRank);

      /*printf("Rank id %4d (%s) communicated with rank id %4d (%s)\n", 
             i, (procNames+i*MPI_MAX_PROCESSOR_NAME), 
             currtarg, (procNames+currtarg*MPI_MAX_PROCESSOR_NAME));*/
    }
    putchar('\n');
  }
}


void listRankLocations(int rank, int wsize)
{
  char *procNames;
  char myProcName[MPI_MAX_PROCESSOR_NAME];
  int nameSize, i;

  if ( rank == 0 )
    procNames = (char*)malloc(wsize*MPI_MAX_PROCESSOR_NAME*sizeof(char));

  MPI_Get_processor_name(myProcName, &nameSize);

  MPI_Gather(myProcName, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, procNames, 
             MPI_MAX_PROCESSOR_NAME, MPI_CHAR, 0, MPI_COMM_WORLD);

  if ( rank == 0 )
  {
    putchar('\n');

    for ( i = 0; i < wsize; i++ )
    {
      printf("Rank id %4d ran on %s\n", 
             i, (procNames+i*MPI_MAX_PROCESSOR_NAME));
    }
    putchar('\n');
  }
}


void printReportFooter(double mintime, int rank, int wsize, 
                       int procsPerNode, int useNearestRank)
{
  double wtick, wtoverhead;

  printCommTargets(rank, wsize, procsPerNode, useNearestRank);
  
  if ( rank == 0 )
  {
    wtick      = MPI_Wtick();
    wtoverhead = getWtimeOh();

    putchar('\n');
    printf("MPI_Wtick returns           %11.9f\n", wtick);
    printf("MPI_Wtime overhead          %11.9f\n", wtoverhead);
    printf("Ticks for minimum sample    %11.0f\n\n", mintime/wtick);
  }
}


void getPair(int idx, int wsize, int procsPerNode, 
             char allocPattern, int useNearestRank,
             int *rank1, int *rank2)
{
  if ( allocPattern == 'c' && procsPerNode > 0 && wsize > 2*procsPerNode )
  {

    if ( useNearestRank )
    {
      int calcval;

      calcval = idx * procsPerNode * 2;
      *rank1 = calcval % wsize + calcval/wsize;
    }
    else
    {
      int calcval;

      calcval = idx * procsPerNode;
      *rank1 = calcval % (wsize/2) + calcval/(wsize/2);
    }
      
  }
  else
  {
    if ( useNearestRank )
      *rank1 = idx + (idx/procsPerNode * procsPerNode);
    else
      *rank1 = idx;
  }

  *rank2 = getTargetRank(*rank1, wsize, procsPerNode, useNearestRank);
}


int getTargetRank(int rank, int wsize, int procsPerNode, int useNearestRank)
{
  int target = -1;

  if ( useNearestRank && procsPerNode > 0 )
  {
    if ( rank % (procsPerNode*2) >= procsPerNode )
      target = rank - procsPerNode;
    else
      target = rank + procsPerNode;
  }
  else
  {
    if ( rank < wsize/2 )
      target = rank + wsize/2;
    else
      target = rank - wsize/2;
  }

  return target;
}


void createActiveComm(int procs, int rank, int wsize, int procsPerNode, 
                         char allocPattern, int printPairs,
                         int useNearestRank, MPI_Comm *activeComm)
{
  int *activeIds, limit;
  int i;
  MPI_Group worldGroup, activeGroup;

  MPI_Comm_group(MPI_COMM_WORLD, &worldGroup);
  
  /*  Create Communicator of all active processes  */
  activeIds = (int*)malloc(procs*sizeof(int));

  limit = procs/2;
  
  for ( i = 0; i < limit; i++ )
  {
    getPair(i, wsize, procsPerNode, allocPattern, useNearestRank,
            &(activeIds[i]), &(activeIds[limit+i]));
  }

  MPI_Group_incl(worldGroup, procs, activeIds, &activeGroup);
  MPI_Comm_create(MPI_COMM_WORLD, activeGroup, activeComm);
  free(activeIds);
  MPI_Group_free(&worldGroup);
  MPI_Group_free(&activeGroup);
}


void printActivePairs(int procs, int rank, int wsize, int procsPerNode, 
                      char allocPattern, int useNearestRank)
{
  int i, cp1, cp2;

  if ( rank == 0 )
  {
    if ( allocPattern == 'c' && procsPerNode > 0 && wsize > procsPerNode*2 )
      printf("\nCurrent pairs, cyclic allocation\n");
    else
      printf("\nCurrent pairs, block allocation\n");

    for ( i = 0; i < procs/2; i++ )
    {
      getPair(i, wsize, procsPerNode, allocPattern, useNearestRank,
              &cp1, &cp2);
      
      printf("  %5d:%5d\n", cp1, cp2);
    }
    printf("\n");
  }
}


int isActiveProc(int rank, int wsize, int procsPerNode, int count, 
                 char allocPattern, int useNearestRank)
{
  int i, cp1, cp2;

  for ( i = 0; i < count/2; i++ )
  {
    getPair(i, wsize, procsPerNode, allocPattern, useNearestRank,
            &cp1, &cp2);

    if ( rank == cp1 || rank == cp2 )
      return 1;
  }

  return 0;
}


int processArgs(int argc, char **argv, int rank, int wsize, int *iters, 
                int *eiters, int *messStart, int *messStop, int *messFactor, 
                char **procFile, int *procsPerNode, 
                char *allocPattern, int *printPairs, int *useBarrier,
                int *useNearestRank)
{
  int i;
  char *argptr;
  char flag;

  *iters           = 0;
  *eiters          = 0;
  *messStart       = MESS_START_DEF;
  *messStop        = MESS_STOP_DEF;
  *messFactor      = MESS_FACTOR_DEF;
  *procFile        = NULL;
  *procsPerNode    = 0;
  *allocPattern    = ALLOC_DEF;
  *useBarrier      = USE_BARRIER_DEF;
  *printPairs      = PRINT_PAIRS_DEF;
  *useNearestRank  = NEAREST_RANK_DEF;

  for ( i = 0; i < argc; i++ )
  {
    if ( argv[i][0] == '-' )
    {
      flag   = argv[i][1];
      argptr = NULL;

      if ( !strchr("inbcefhoprst", flag) && rank == 0 )
      {
        printf("\nInvalid flag -%c!\n", flag);
        return(0);
      }
      
      switch(flag)
      {
      case 'i':
        *printPairs = 1;
        break;
      case 'n':
        *useBarrier = 0;
        break;
      case 'r':
        *useNearestRank = 1;
        break;
      default:
        if ( argv[i][2] != 0 )
          argptr = &(argv[i][2]);
        else
        {
          argptr = argv[i+1];
          i++;
        }
        break;
      }

      switch(flag)
      {
      case 'b':
        *messStart = atoi(argptr);
        break;
      case 'c':
        *eiters = atoi(argptr);
        break;
      case 'e':
        *messStop = atoi(argptr);
        break;
      case 'f':
        *procFile = strdup(argptr);
        break;
      case 'h':
        return(0);
      case 'o':
        *iters = atoi(argptr);
        break;
      case 'p':
        *allocPattern = *argptr;
        break;
      case 's':
        *messFactor = atoi(argptr);
        break;
      case 't':
        *procsPerNode = atoi(argptr);
        break;
      }
    }
  }

  if ( *iters == 0 )
  {
    if ( rank == 0 )
      printf("\n  Must define number of operations per measurement!\n\n");

    return(0);
  }

  if ( *allocPattern == 'c' && procsPerNode == 0 )
  {
    if ( rank == 0 )
      printf("\n  Cyclic allocation requires a process per node argument!\n\n");
    return(0);
  }

  if ( *allocPattern == 'c' && wsize <= *procsPerNode*2 )
  {
    if ( rank == 0 )
    {
      printf("\n  Cyclic allocation can only be used for jobs\n");
      printf("    with MPI process count > 2*processes/node.\n\n");

    }
    return(0);
  }

  return(1);
}


int getProcList(char* procFile, int wsize, int** procListPtr, int* listSizePtr,
                int procsPerNode, char allocPattern)
{
  FILE* fp     = NULL;
  char lineBuf[1024];
  int currProcCount;
  int* procList;
  int listSize = 0;
  int nodes;

  *procListPtr = NULL;
  *listSizePtr = 0;

  if ( procFile != NULL )
  {
    if ( (fp = fopen(procFile, "r")) != NULL )
    {
      procList = (int*)malloc(1024*sizeof(int));
      if ( procList == NULL )
      {
        fclose(fp);
        return 0;
      }
  
      while ( fgets(lineBuf, 1024, fp) != NULL )
      {
        currProcCount = atoi(lineBuf);
        if ( currProcCount > 1 && currProcCount % 2 == 0 && 
             currProcCount <= wsize)
        {
          procList[listSize] = atoi(lineBuf);
  	  listSize++;
  	  if ( listSize % 1024 == 0 )
  	  {
            procList = (int*)realloc(procList, (listSize/1024+1)*1024*sizeof(int));
            if ( procList == NULL )
            {
              fclose(fp);
              return 0;
            }
          }
        }
      }
  
      fclose(fp);
    }
    else
    {
      return 0;
    }
  }
  else
  {
    int count, i;

    if ( procsPerNode > 0 && wsize > procsPerNode*2 )
    {
      if ( allocPattern == 'c' )
      {
        nodes = wsize/procsPerNode;

        for ( count = nodes; count <= wsize; count += nodes )
          listSize++;
      }
      else
      {
        for ( count = procsPerNode*2; count <= wsize; count *= 2 )
          listSize++;

        if ( count/2 != wsize )   /* Check if MPI_COMM_WORLD not power of 2 */
          listSize++;
      }
    }
    else
    {
      for ( count = 2; count <= wsize; count *= 2 )
        listSize++;

      if ( count/2 != wsize )   /* Check if MPI_COMM_WORLD is not power of 2 */
        listSize++;
    }

    procList = (int*)malloc(listSize*sizeof(int));
    if ( procList == NULL )
      return 0;

    if ( procsPerNode > 0 && wsize > procsPerNode*2 )
    {
      if ( allocPattern == 'c' )
      {
        for ( count = nodes, i = 0; count <= wsize; count += nodes, i++ )
          procList[i] = count;
      }
      else
      {
        for ( count = (procsPerNode*2), i = 0; count <= wsize; 
              count *= 2, i++ )
        {
          procList[i] = count;
        }

        if ( count/2 != wsize )  /* Check if MPI_COMM_WORLD not power of 2 */
          procList[i] = wsize;
      }
    }
    else
    {
      for ( count = 2, i = 0; count <= wsize; count *= 2, i++ )
        procList[i] = count;

      if ( count/2 != wsize )   /* Check if MPI_COMM_WORLD is not power of 2 */
        procList[i] = wsize;
    }
  }

  *procListPtr = procList;
  *listSizePtr = listSize;

  return 1;
}
