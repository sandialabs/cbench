#ifdef MPI
#include <stdlib.h>
#include <errno.h>

#include "prctim.h"
#include "piob.h"
#include "xprt_mpi.h"

static int initialized = 0;

MPI_Datatype filargs_type;
MPI_Datatype xfrparms_type;
MPI_Datatype precise_time_type;
MPI_Datatype metric_type;

static unsigned _rank;
static unsigned _num_ranks;

/*
 * MPI_Address yields address offsets. We will need relative displacements for
 * the MPI_Type_struct calls. This macro will convert the entire vector.
 */
#define cvt2disp(base, d, dlen) \
	do { \
		int	i; \
 \
		for (i = (dlen) - 1; i >= 0; i++) \
			(d)[i] -= (base); \
	} while (0)

static void
build_filargs_type(void)
{
	MPI_Datatype type[3] = {MPI_INT, MPI_INT, MPI_CHAR};
	int	blocklen[3] = {1, 1, PIOB_MAX_PATHLEN};
	MPI_Aint base, disp[3];
	struct filargs *argp = (struct filargs *)0;
	int	result;

	result = MPI_Address(argp, &base);
	if (result != MPI_SUCCESS)
		fatal_mpi("Address[filargs+base]");
	result = MPI_Address(&argp->mode, &disp[0]);
	if (result != MPI_SUCCESS)
		fatal_mpi("Address[filargs+mode]");
	result = MPI_Address(&argp->new, &disp[1]);
	if (result != MPI_SUCCESS)
		fatal_mpi("Address[filargs+new]");
	result = MPI_Address(argp->path, &disp[2]);
	if (result != MPI_SUCCESS)
		fatal_mpi("Address[filargs+path]");

	result = MPI_Type_struct(3, blocklen, disp, type, &filargs_type);
	if (result != MPI_SUCCESS)
		fatal_mpi("Type_struct[filargs]");
	result = MPI_Type_commit(&filargs_type);
	if (result != MPI_SUCCESS)
		fatal_mpi("Type_commit[filargs]");
}

static void
build_xfrparms_type(void)
{
	MPI_Datatype type[3] =
	    {
		MPI_UNSIGNED_LONG,
		MPI_UNSIGNED_LONG,
		MPI_UNSIGNED_LONG
	    };
	int	blocklen[3] = {1, 1, 1};
	MPI_Aint base, disp[3];
	struct xfrparms *argp = (struct xfrparms *)0;
	int	result;

	result = MPI_Address(argp, &base);
	if (result != MPI_SUCCESS)
		fatal_mpi("Address[xfrparms+base]");
	result = MPI_Address(&argp->xlen, &disp[0]);
	if (result != MPI_SUCCESS)
		fatal_mpi("Address[xfrparms+xlen]");
	result = MPI_Address(&argp->nx, &disp[1]);
	if (result != MPI_SUCCESS)
		fatal_mpi("Address[xfrparms+nx]");
	result = MPI_Address(&argp->step, &disp[2]);
	if (result != MPI_SUCCESS)
		fatal_mpi("Address[xfrparms+step]");

	result = MPI_Type_struct(3, blocklen, disp, type, &xfrparms_type);
	if (result != MPI_SUCCESS)
		fatal_mpi("Type_struct[xfrparms]");
	result = MPI_Type_commit(&xfrparms_type);
	if (result != MPI_SUCCESS)
		fatal_mpi("Type_commit[xfrparms]");
}

static void
build_precise_time_type(void)
{
	MPI_Datatype type[2] = {MPI_UNSIGNED_LONG, MPI_UNSIGNED_LONG};
	int	blocklen[2] = {1, 1};
	MPI_Aint base, disp[2];
	struct precise_time *argp = (struct precise_time *)0;
	int	result;

	result = MPI_Address(argp, &base);
	if (result != MPI_SUCCESS)
		fatal_mpi("Address[precise_time+base]");
	result = MPI_Address(&argp->pt_sec, &disp[0]);
	if (result != MPI_SUCCESS)
		fatal_mpi("Address[precise_time+pt_sec]");
	result = MPI_Address(&argp->pt_nsec, &disp[1]);
	if (result != MPI_SUCCESS)
		fatal_mpi("Address[precise_time+pt_nsec]");

	result = MPI_Type_struct(2, blocklen, disp, type, &precise_time_type);
	if (result != MPI_SUCCESS)
		fatal_mpi("Type_struct[precise_time]");
	result = MPI_Type_commit(&precise_time_type);
	if (result != MPI_SUCCESS)
		fatal_mpi("Type_commit[precise_time]");
}

static void
build_metric_type(void)
{
	MPI_Datatype type[2] = {MPI_INT, precise_time_type};
	int	blocklen[2] = {1, 1};
	MPI_Aint base, disp[2];
	struct metric *argp = (struct metric *)0;
	int	result;

	result = MPI_Address(argp, &base);
	if (result != MPI_SUCCESS)
		fatal_mpi("Address[metric+base]");
	result = MPI_Address(&argp->err, &disp[0]);
	if (result != MPI_SUCCESS)
		fatal_mpi("Address[metric+rtn]");
	result = MPI_Address(&argp->elapsed, &disp[1]);
	if (result != MPI_SUCCESS)
		fatal_mpi("Address[metric+elapsed]");

	result = MPI_Type_struct(2, blocklen, disp, type, &metric_type);
	if (result != MPI_SUCCESS)
		fatal_mpi("Type_struct[metric]");
	result = MPI_Type_commit(&metric_type);
	if (result != MPI_SUCCESS)
		fatal_mpi("Type_commit[metric]");
}

void
msg_init(int *argc, char *const *argv[])
{
	int	err;
	int	i;

	if (initialized)
		return;
	initialized = 1;

	err = MPI_Init(argc, (char ***)argv);
	if (err != MPI_SUCCESS)
                fatal_mpi("Init");

	err = MPI_Comm_rank(MPI_COMM_WORLD, &i);
	if (err != MPI_SUCCESS)
		fatal_mpi("Comm_rank");
#ifdef PARANOID
	if (i < 0)
		fatal("MPI returned negative rank!");
#endif
	_rank = (unsigned )i;

	if (MPI_Comm_size(MPI_COMM_WORLD, &i) != MPI_SUCCESS)
		fatal_mpi("Comm_size");
#ifdef PARANOID
	if (i < 0)
		fatal("MPI returned negative number of ranks!");
#endif
	_num_ranks = i;

	build_filargs_type();
	build_xfrparms_type();
	build_precise_time_type();
	build_metric_type();
}

unsigned
self(void)
{

	if (!initialized)
		fatal("self -- mpi transport not initialized");

	return _rank;
}

unsigned
nrank(void)
{

	if (!initialized)
		fatal("nrank -- mpi transport not initialized");

	return _num_ranks;
}

void
all_abort(int rtn)
{

	(void )MPI_Abort(MPI_COMM_WORLD, rtn);
	exit(1);
}
#endif /* defined(MPI) */
