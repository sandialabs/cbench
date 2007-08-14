/*
 * The master rank identifier.
 */
#define PIOB_MASTER_RANK	0

/*
 * Test if the passed rank ID is the master's.
 */
#define is_master(r)		((r) == PIOB_MASTER_RANK)

/*
 * Slave-supported tasks.
 */
typedef enum {
	TASK_SET_PARAMETERS = 0,
	TASK_SET_FILE = 1,
	TASK_WRITES = 2,
	TASK_READS = 3,
	TASK_CLOSE = 4,
	TASK_TERMINATE = 5,
	TASK_INVALIDATE = 6
} t_t;

/*
 * Maximum path length supported.
 */
#define PIOB_MAX_PATHLEN	8192

/*
 * File arguments
 */
struct filargs {
	int	mode;					/* 0, 1 or 2 */
	int	new;					/* new file? */
	char	path[PIOB_MAX_PATHLEN + 1];		/* path name */
};

/*
 * Transfer parameters.
 */
struct xfrparms {
	unsigned long xlen;				/* transfer length */
	unsigned long nx;				/* number transfers */
	unsigned long step;				/* step size */
};

/*
 * IO task results
 */
struct metric {
	int	err;					/* error code */
	struct precise_time elapsed;			/* elapsed */
};

/*
 * A union of all arguments used in send/receives.
 */
union all_args {
	int	aa_inum;
	struct filargs aa_filargs;
	struct xfrparms aa_xfrparms;
};

/*
 * Union of all results used in send/recieves.
 */
union all_results {
	int	ar_inum;
	struct metric ar_metric;
};

extern int debugging;

extern int run;

#ifdef __GNUC__
#define IS_UNUSED __attribute__ ((unused))
#else
#define IS_UNUSED
#endif

extern int set_parameters(unsigned, unsigned, unsigned);
extern int set_file(const char *, int, int);
extern int set_file_by_map(const char *, const size_t *, int, int);
extern int writes(int, struct precise_time *);
extern int reads(int, struct precise_time *);
extern int close_file(int);
extern int terminate(void);
extern int invalidate(void);

extern void perform_set_parameters(struct xfrparms *, int *);
extern void perform_set_file(struct filargs *, int *);
extern void perform_writes(int *, struct metric *);
extern void perform_reads(int *, struct metric *);
extern void perform_close_file(int *, int *);
extern void perform_terminate(void *, int *result);
extern void perform_invalidate(void *, int *result);

extern void msg(const char *, const char *, ...);
extern void error(const char *, ...);
extern void fatal(const char *, ...);

extern unsigned nrank(void);
extern unsigned self(void);
extern void all_abort(int);
extern void msg_init(int *, char *const *[]);
