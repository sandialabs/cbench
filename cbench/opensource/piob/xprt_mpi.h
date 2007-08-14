#ifdef MPI
#include <mpi.h>

/*
 * MPI call failed.
 */
#define fatal_mpi(s) \
	do { \
		fatal("MPI_%s: call failed", (s)); \
	} while (0)

extern MPI_Datatype xfrparms_type;
extern MPI_Datatype filargs_type;
extern MPI_Datatype metric_type;

extern void dispatch(int, void *, void *);
extern void slave(void);
#endif /* defined(MPI) */
