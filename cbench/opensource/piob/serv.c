#ifdef MPI
#include <stdio.h>					/* for NULL :-( */

#include "prctim.h"
#include "piob.h"
#include "xprt_mpi.h"

/*
 * Union of all possible call arguments.
 */
static union {
	int	aa_inum;
	struct xfrparms aa_xfrparms;
	struct filargs aa_filargs;
} all_args;						/* call args */

/*
 * Union of all possible replies.
 */
static union {
	int	ar_inum;
	struct metric ar_metric;
} all_results;						/* reply buffer */

static MPI_Datatype _mpi_int_type = MPI_INT;

/*
 * RPC envelope attributes.
 */
static struct attrs {
	const char *name;				/* task name */
	t_t	tag;					/* task tag */
	int	rcount;					/* rcv count */
	MPI_Datatype *rtyp;				/* rcv type */
	int	scount;					/* send count */
	MPI_Datatype *styp;				/* send type */
	void	(*proc)(void *, void *);		/* procedure */
} pattrs[] = {						/* proc attrs */
	{
		"SET_PARAMETERS",
		TASK_SET_PARAMETERS,
		1,
		&xfrparms_type,
		1,
		&_mpi_int_type,
		(void (*)(void *, void *))perform_set_parameters
	},
	{
		"SET_FILE",
		TASK_SET_FILE,
		1,
		&filargs_type,
		1,
		&_mpi_int_type,
		(void (*)(void *, void *))perform_set_file
	},
	{
		"WRITES",
		TASK_WRITES,
		1,
		&_mpi_int_type,
		1,
		&metric_type,
		(void (*)(void *, void *))perform_writes
	},
	{
		"READS",
		TASK_READS,
		1,
		&_mpi_int_type,
		1,
		&metric_type,
		(void (*)(void *, void *))perform_reads
	},
	{
		"CLOSE",
		TASK_CLOSE,
		1,
		&_mpi_int_type,
		1,
		&_mpi_int_type,
		(void (*)(void *, void *))perform_close_file
	},
	{
		"TERMINATE",
		TASK_TERMINATE,
		0,
		&_mpi_int_type,
		1,
		&_mpi_int_type,
		(void (*)(void *, void *))perform_terminate
	},
#ifdef INVALIDATE_CACHED_DATA
	{
		"INVALIDATE",
		TASK_INVALIDATE,
		0,
		&_mpi_int_type,
		1,
		&_mpi_int_type,
		(void (*)(void *, void *))perform_invalidate
	},
#endif
	{
		"<dummy>",
		-1,
		0,
		&_mpi_int_type,
		0,
		&_mpi_int_type,
		NULL
	}
};

void
dispatch(int tag, void *arg, void *result)
{
	struct attrs *pa;
	
	if (tag < 0 || (unsigned )tag > (sizeof(pattrs) / sizeof(struct attrs)))
		fatal("dispatch called with bad tag (%d)", tag);
	pa = &pattrs[tag];
	if ((int )pa->tag != tag)
		fatal("illegal tag (%d)", tag);
	if (debugging)
		msg("DEBUG", "rcv'd rqst %s", pa->name);

	(*pa->proc)(arg, result);
}

void
slave(void)
{
	int	err;
	MPI_Status status;
	struct attrs *pa;

	if (debugging)
		msg("DEBUG", "as slave");

	/*
	 * The receive, dispatch, reply loop.
	 */
	while (run) {
		/*
		 * Probe for incoming message type.
		 */
		err =
		    MPI_Probe(PIOB_MASTER_RANK,
			      MPI_ANY_TAG,
			      MPI_COMM_WORLD,
			      &status);
		if (err != MPI_SUCCESS || status.MPI_ERROR != MPI_SUCCESS)
			fatal_mpi("Probe");

		if (status.MPI_TAG < 0 ||
		    ((unsigned )status.MPI_TAG >
		     (sizeof(pattrs) / sizeof(struct attrs))))
			fatal("Protocol error -- bad tag (%d)", status.MPI_TAG);

		/*
		 * Get attributes of incoming message.
		 */
		pa = &pattrs[status.MPI_TAG];

		/*
		 * Receive the message.
		 */
		err =
		    MPI_Recv(&all_args,
			     pa->rcount,
			     *pa->rtyp,
			     PIOB_MASTER_RANK,
			     status.MPI_TAG,
			     MPI_COMM_WORLD,
			     &status);
		if (err != MPI_SUCCESS ||
		    status.MPI_ERROR != MPI_SUCCESS)
			fatal_mpi("Recv");

		/*
		 * Call dispatcher to perform the function.
		 */
		dispatch(status.MPI_TAG, &all_args, &all_results);

		/*
		 * If we are to reply, do so now.
		 */
		if (pa->scount) {
			err =
			    MPI_Send(&all_results,
				     pa->scount,
				     *pa->styp,
				     PIOB_MASTER_RANK,
				     MPI_TAG_UB,
				     MPI_COMM_WORLD);
			if (err != MPI_SUCCESS)
				fatal_mpi("Send");
		}
	}
}
#endif /* defined(MPI) */
