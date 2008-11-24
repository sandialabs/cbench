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


    util.h

*/

#define MESS_START_DEF 32
#define MESS_STOP_DEF 8388608
#define MESS_FACTOR_DEF 2
#define ALLOC_DEF 'b'
#define USE_BARRIER_DEF 1
#define PRINT_PAIRS_DEF 0
#define NEAREST_RANK_DEF 0

double getWtimeOh();
void populateData(char*, int);
void generic_barrier(MPI_Comm);
void printReportFooter(double mintime, int rank, int wsize, int procsPerNode, int useNearestRanks);
void printCommTargets(int rank, int wsize, 
                      int procsPerNode, int useNearestRank);
void listRankLocations(int rank, int wsize);
void createActiveComm(int procs, int rank, int wsize, int procsPerNode, 
                      char allocPattern, int printPairs,
                      int useNearestRank, MPI_Comm *activeComm);
int isActiveProc(int rank, int wsize, int procsPerNode, 
                 int count, char allocPattern, int useNearestRank);
int processArgs     (int argc, char **argv, int rank, int wsize, int *iters, 
                     int *eiters, int *messStart, int *messStop, 
                     int *messFactor, char **procFile, int *procsPerNode, 
                     char *allocPattern, int *printPairs, int *useBarrier,
                     int *useNearestRank);
int getProcList     (char* procFile, int wsize, 
                     int** procListPtr, int* listSizePtr,
                     int procsPerNode, char allocPattern);
void printActivePairs(int procs, int rank, int wsize, int procsPerNode, 
                      char allocPattern, int useNearestRank);
int getTargetRank(int rank, int wsize, int procsPerNode, int useNearestRank);
void getPair(int idx, int wsize, int procsPerNode, 
             char allocPattern, int useNearestRank,
             int *rank1, int *rank2);
