#ifdef MPI

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "prctim.h"
#include "piob.h"
#include "xprt_mpi.h"

#if PIOB_MASTER_RANK != 0
/*
 * The task all routine uses MPI_Wait in order to complete the
 * pre-posted replies. Though my book doesn't say one way or the other
 * the MPICH implementation doesn't like a request to be MPI_REQUEST_NULL.
 * This implementation assumes the master rank index is zero. It uses that
 * assumption to avoid waiting on the master, which had the send/recieve
 * short-circiuted.
 */
#error FIX ME! task_all only works when master is rank 0
#endif

/*
 * Tasks names (for debuggingging)
 */
static const char *task_names[] = {
	"SET_PARAMETERS",
	"SET_FILE",
	"WRITES",
	"READS",
	"CLOSE",
	"TERMINATE",
	"INVALIDATE",
};

void
task_all(void *sbuf, MPI_Datatype styp, size_t ssiz, unsigned step,
	 int tag,
	 void *rvec, MPI_Datatype rtyp, size_t rsiz,
	 int delay)
{
	unsigned myid;
	unsigned n;
	MPI_Request *rqvec, *rqst;
	MPI_Status *stvec, *st;
	unsigned dst;
	void	*rv;
	int	err;

	if (tag < 0 ||
	    (unsigned )tag >= sizeof(task_names) / sizeof(const char *))
		fatal_mpi("Bad/illegal tag");
	if (debugging)
		msg("DEBUG", "task all %s", task_names[tag]);

	myid = self();
	n = nrank();

	/*
	 * Synchronous calls will supply reply space. Prepost
	 * those now.
	 */
	rqvec = NULL;
	stvec = NULL;
	if (rvec != NULL) {
		rqvec = malloc(n * sizeof(MPI_Request));
		if (rqvec == NULL)
			fatal("can't alloc rqvec");
		stvec = malloc(n * sizeof(MPI_Status));
		if (stvec == NULL)
			fatal("can't alloc stvec");
		err = 0;
		for (dst = 0, rv = rvec, rqst = rqvec;
		     dst < n;
		     dst++, (char *)rv += rsiz, rqst++) {
			*rqst = MPI_REQUEST_NULL;
			if (dst == myid) {
				continue;		/* not caller! */
			}
			err =
			    MPI_Irecv(rv,
				      1,
				      rtyp,
				      (int )dst,
				      MPI_TAG_UB,
				      MPI_COMM_WORLD,
				      rqst);
			if (err != MPI_SUCCESS)
				fatal_mpi("Irecv");
#ifdef PARANOID
			if (*rqst == MPI_REQUEST_NULL)
				fatal("MPI blew the Irecv");
#endif
		}
	}

	/*
	 * Send the requests.
	 */
	for (dst = 0; dst < n; dst++) {
		if (dst == myid)
			continue;			/* not caller! */
		err =
		    MPI_Send((char *)sbuf + (dst * step * ssiz),
			     sbuf != NULL ? 1 : 0,
			     styp,
			     dst,
			     tag,
			     MPI_COMM_WORLD);
		if (err != MPI_SUCCESS)
			fatal_mpi("Send");
		if (delay)
			sleep(delay);
	}

	dispatch(tag,
		 (char *)sbuf + (myid * step *ssiz),
		 (char *)rvec + (myid * rsiz));

	if (rvec != NULL) {
		/*
		 * Collect responses from the servers/slaves.
		 */
		err = MPI_Waitall(n - 1, rqvec + 1, stvec + 1);
		if (err != MPI_SUCCESS)
			fatal_mpi("Waitall");

		for (dst = 0, rqst = rqvec, st = stvec;
		     dst < n;
		     dst++, rqst++, st++) {
			if (*rqst != MPI_REQUEST_NULL)
				fatal("missed reply from %d", dst);
			if (dst == myid)
				continue;		/* skip caller! */
			if (debugging)
				msg("DEBUG",
				    "rcvd reply from %d (%d)",
				    dst,
				    st->MPI_ERROR);
			if (st->MPI_ERROR != MPI_SUCCESS)
				fatal_mpi("Irecv[completion]");
		}
	}
}


static int
collect_result(int *result)
{
	unsigned n;
	unsigned i;
	int	*r;
	int	rtn;

	n = nrank();
	rtn = 0;
	for (i = 0, r = result; i < n; i++, r++) {
		if (i == self())
			continue;
		if (*r)
			rtn = -1;
	}

	return rtn;
}

int
set_parameters(unsigned xlen, unsigned nx, unsigned step)
{
	struct xfrparms arg;
	int	*result;
	int	err;

	arg.xlen = xlen;
	arg.nx = nx;
	arg.step = step;

	result = malloc(sizeof(int) * nrank());
	if (result == NULL)
		fatal("set_parameters: can't alloc results");

	task_all(&arg, xfrparms_type, sizeof(struct xfrparms), 0,
		 TASK_SET_PARAMETERS,
		 result, MPI_INT, sizeof(int),
		 0);

	err = collect_result(result);
	free(result);
	return err;
}

int
set_file(const char *path, int mode, int new)
{
	struct filargs *arg;
	int	*result;
	int	err;

	if (strlen(path) >= sizeof(arg->path))
		fatal("set_file: path name too long");

	arg = malloc(sizeof(struct filargs));
	if (arg == NULL)
		fatal("set_file: can't alloc arg");
	arg->mode = mode;
	arg->new = new;
	(void )strcpy(arg->path, path);

	result = malloc(sizeof(int) * nrank());
	if (result == NULL)
		fatal("set_file: can't alloc results");

	task_all(arg, filargs_type, sizeof(struct filargs), 0,
		 TASK_SET_FILE,
		 result, MPI_INT, sizeof(int),
		 0);

	err = collect_result(result);
	free(result);
	free(arg);
	return err;
}

/*
 * Set file names for the slaves from a passed map.
 *
 * This is *so* naive. The argument uses a fixed length buffer to pass
 * the path name. With very large machines, this is megabytes of memory
 * to set up the arguments. A new implementation should be considered. Perhaps,
 * a new call that sets the file name, followed by a call to open the file
 * named.
 */
int
set_file_by_map(const char *map, const size_t *offsets, int mode, int new)
{
	struct filargs *argvec;
	unsigned n;
	const char *path;
	int	*result;
	int	err;

	argvec = malloc(nrank() * sizeof(struct filargs));
	if (argvec == NULL)
		fatal("set_file_by_map: can't alloc arg vec");
	for (n = 0; n < nrank(); n++) {
		path = map + *(offsets++);
		if (strlen(path) >= sizeof(argvec[n].path))
			fatal("set_file_by_map: path name too long");
		argvec[n].mode = mode;
		argvec[n].new = new;
		(void )strcpy(argvec[n].path, path);
	}

	result = malloc(sizeof(int) * nrank());
	if (result == NULL)
		fatal("set_file_by_map: can't alloc results");

	task_all(argvec, filargs_type, sizeof(struct filargs), 1,
		 TASK_SET_FILE,
		 result, MPI_INT, sizeof(int),
		 0);

	err = collect_result(result);
	free(result);
	free(argvec);
	return err;
}
int
writes(int locking, struct precise_time *elapsed)
{
	int	n;
	struct metric *result, *r;
	int	err;
	int	i;

	n = nrank();
	result = malloc(sizeof(struct metric) * n);
	if (result == NULL)
		fatal("writes: can't alloc results");

	task_all(&locking, MPI_INT, sizeof(int), 0,
		 TASK_WRITES,
		 result, metric_type, sizeof(struct metric),
		 0);

	err = 0;
	for (i = 0, r = result; i < n; i++, r++) {
		if (r->err)
			err = -1;
		*elapsed++ = r->elapsed;
	}
	free(result);
	return err;
}

int
reads(int locking, struct precise_time *elapsed)
{
	int	n;
	struct metric *result, *r;
	int	err;
	int	i;

	n = nrank();
	result = malloc(sizeof(struct metric) * n);
	if (result == NULL)
		fatal("reads: can't alloc results");

	task_all(&locking, MPI_INT, sizeof(int), 0,
		 TASK_READS,
		 result, metric_type, sizeof(struct metric),
		 0);

	err = 0;
	for (i = 0, r = result; i < n; i++, r++) {
		if (r->err)
			err = -1;
		*elapsed++ = r->elapsed;
	}
	free(result);
	return err;
}

int
close_file(int keep)
{
	int	*result;
	int	err;

	result = malloc(sizeof(int) * nrank());
	if (result == NULL)
		fatal("set_parameters: can't alloc results");

	if (!keep && nrank() > 1)
		msg("INFO",
		    "Please wait (%d seconds) while I clean up...",
		    nrank());
	task_all(&keep, MPI_INT, sizeof(int), 0,
		 TASK_CLOSE,
		 result, MPI_INT, sizeof(int),
		 keep ? 0 : 1);

	err = collect_result(result);
	free(result);
	return err;
}

int
terminate(void)
{
	int	*result;
	int	err;

	result = malloc(sizeof(int) * nrank());
	if (result == NULL)
		fatal("set_parameters: can't alloc results");

	task_all(NULL, MPI_INT, 0, 0,
		 TASK_TERMINATE,
		 result, MPI_INT, sizeof(int),
		 0);

	err = collect_result(result);
	free(result);
	return err;
}

#ifdef INVALIDATE_CACHED_DATA
int
invalidate(void)
{
	int	*result;
	int	err;

	result = malloc(sizeof(int) * nrank());
	if (result == NULL)
		fatal("set_parameters: can't alloc results");

	task_all(NULL, MPI_INT, 0, 0,
		 TASK_INVALIDATE,
		 result, MPI_INT, sizeof(int),
		 0);

	err = collect_result(result);
	free(result);
	return err;
}
#endif /* defined(INVALIDATE_CACHED_DATA) */
#endif /* defined(MPI) */
