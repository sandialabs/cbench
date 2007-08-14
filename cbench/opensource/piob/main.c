#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#ifdef MPI
#include "xprt_mpi.h"
#endif

#include "prctim.h"
#include "piob.h"

int	debugging = 0;					/* debugging? */
int	verbose = 0;					/* verbose? */
int	uniq = 0;					/* file per proc? */
int	overlap = 0;					/* overlap IO? */
unsigned rotate = 0;					/* rotate IO ops? */
int	locking = 0;					/* use region locks? */

int	run = 1;					/* running? */

static unsigned long minxlen;				/* min xfr len */
static unsigned long maxxlen;				/* max xfr len */
static unsigned long stepx = 1;				/* step */

static unsigned long file_size;				/* trgt file size */
static const char *file_name = NULL;			/* scratch file name */

static char *map = NULL;				/* rf map buffer */
static size_t *map_offsets = NULL;			/* rf map entry off */
static size_t *rd_offsets = NULL;			/* rd map entry off */
static size_t *wr_offsets = NULL;			/* wr map entry off */
static size_t map_entry_count = 0;			/* rf map ent count */

static void fini(int);
static void usage(void);
static void get_range_parameter(const char *);
static void display_results(const char *,
			    struct xfrparms *,
			    struct precise_time *);
static void get_file_map(char *const);

int
main(int argc, char *const argv[])
{
	int	i;
	char	*s, *cp;
	int	err;
	int	fd;
	struct precise_time *results;
	struct xfrparms parms;
	unsigned long n;
	unsigned long u;

#ifdef MPI
	msg_init(&argc, &argv);
#endif /* defined(MPI) */


	/*
	 * Parse options.
	 */
	while ((i = getopt(argc, argv, "dvuor:l")) != -1)
		switch (i) {

		case 'd':				/* debugging? */
			debugging = 1;
			break;
		case 'v':				/* debugging? */
			verbose = 1;
			break;
		case 'u':				/* file per proc? */
			uniq = 1;
			break;
		case 'o':				/* overlap IO? */
			overlap = 1;
			break;
		case 'r':				/* rotate IO ops? */
			rotate = strtoul(optarg, &cp, 0);
			if (*cp != '\0')
				usage();
			break;
		case 'l':				/* region locking? */
			locking = 1;
			break;
		default:
			usage();
		}

	if (debugging)
		msg("DEBUG", "begin");

	/*
	 * There should be three remaining arguments.
	 */
	if (argc - optind != 3)
		usage();

	/*
	 * The slave processes just enter the work loop -- Skipping
	 * command line parsing. Their parameters will be set by
	 * the master, later.
	 */
	if (!is_master(self())) {
		slave();
		exit(0);
	}

	/*
	 * Get the transfer size range.
	 */
	get_range_parameter(argv[optind++]);

	/*
	 * Get the desired scratch file size.
	 */
	s = argv[optind++];
	file_size = strtoul(s, (char **)&cp, 0);
	if (cp == s || (minxlen == ULONG_MAX && errno == ERANGE))
		fatal("bad value for file size");

	/*
	 * Get the scratch file prototype/path or map.
	 */
	if (!uniq) {
		file_name = argv[optind++];
		if (strlen(file_name) >= PIOB_MAX_PATHLEN)
			fatal("scratch file name too long");
	} else {
		get_file_map(argv[optind++]);
		if (map_entry_count < nrank())
			fatal("map file too short (%u/%u)",
			      map_entry_count,
			      nrank());
		/*
		 * Limit the offset to nrank() - 1. Anything larger
		 * is just a modulus.
		 */
		if (rotate > map_entry_count || rotate > nrank() - 1)
			fatal("rotation offset too large");
	}

	wr_offsets = map_offsets;
	if (rotate) {
		rd_offsets = malloc(map_entry_count * sizeof(size_t));
		if (rd_offsets == NULL)
			fatal("can't allocate rd offsets");
		u = nrank();
		for (n = 0; n < u; n++)
			rd_offsets[n] = map_offsets[(n + rotate) % u];
	} else
		rd_offsets = map_offsets;

	/*
	 * Allocate results vector for the IO-related tasks.
	 */
	results = malloc(nrank() * sizeof(struct precise_time));
	if (results == NULL)
		fatal("can't allocate results vector");

	/*
	 * Loop -- Repeating the operations at the various desired
	 * transfer sizes.
	 */
	err = 0;
	for (n = minxlen; n <= maxxlen; n += stepx) {
		parms.xlen = n;
		parms.step = (overlap || uniq) ? 1 : nrank();
		parms.nx = file_size / (n * parms.step);

		err =
		    set_parameters(parms.xlen, parms.nx, parms.step);
		if (err)
			break;

		if (!uniq) {
			fd = creat(file_name, 0666);
			if (fd < 0)
				fatal("%s: %s", file_name, strerror(errno));
			(void )close(fd);

			if ((err = set_file(file_name, 1, 0)))
				break;
		} else if ((err = set_file_by_map(map, wr_offsets, 1, 1)))
			break;

		(void )memset(results,
			      0,
			      nrank() * sizeof(struct precise_time));
		if ((err = writes(locking, results)))
			break;
		display_results("write", &parms, results);

		if ((err = close_file(1)))
			break;

		printf("Sleeping 30 secs...\n");
		sleep(30);

		err =
		    !uniq
		  ? set_file(file_name, 0, 0)
		  : set_file_by_map(map, rd_offsets, 0, 0);
		if (err)
			break;

#ifdef INVALIDATE_CACHED_DATA
		if ((err = invalidate()))
			break;
#endif
		(void )memset(results,
			      0,
			      nrank() * sizeof(struct precise_time));
		if ((err = reads(locking, results)))
			break;
		display_results("read", &parms, results);

		/*
		 * Close the scratch files.
		 */
		if ((err = close_file(uniq ? 0 : 1)))
			break;

		if (!uniq && unlink(file_name) != 0)
			error("%s: %s", file_name, strerror(errno));
	}

	if (terminate() != 0)
		error("terminate failed -- beware hangs");
	if (!run)
		return 0;
	fatal("master failed to terminate");
	return 1;					/* not reached */
}

void
fini(int status)
{

#ifdef MPI
	MPI_Finalize();
#endif

	exit(status);
}

/*
 * Print program usage and exit with a non-zero code.
 */
static void
usage(void)
{

	if (!is_master(self()))
		fini(1);

	(void )fprintf(stderr,
		       "Usage: piob "
		       "[-dvuol] "
		       "[-r <#>] "
		       "<xfr-range> "
		       "<scratch-size> "
		       "<scratch-path or map>\n"
		       "\n"
		       " Where:\n"
		       "\tscratch-file-size is given in bytes\n"
		       "\txfr-range format is <xfr-min>[-<xfr-max>[:<step>]]\n"
		      );

	fini(1);
}

/*
 * Get the transfer range parameters.
 *
 * Format of the argument is:
 *
 * <minlen>[-<maxlen>[:<step size>]]
 *
 * The globals; minxlen, maxxlen, and stepxlen are set as a side-effect.
 */
static void
get_range_parameter(const char *s)
{
	const char *cp;

	stepx = 1;

	errno = 0;
	maxxlen = minxlen = strtoul(s, (char **)&cp, 0);
	if (cp == s || (minxlen == ULONG_MAX && errno == ERANGE))
		fatal("bad value for range parameter");
	if (*cp == '\0')
		return;
	if (*cp != '-')
		usage();
	s = cp + 1;
	errno = 0;
	maxxlen = strtoul(s, (char **)&cp, 0);
	if (cp == s ||
	    (maxxlen == ULONG_MAX && errno == ERANGE) ||
	    maxxlen < minxlen)
		fatal("bad value for max transfer length");
	if (*cp == '\0')
		return;
	if (*cp != ':')
		usage();
	s = cp + 1;
	errno = 0;
	stepx = strtoul(s, (char **)&cp, 0);
	if (cp == s ||
	    (stepx == ULONG_MAX && errno == ERANGE) ||
	    stepx > maxxlen - minxlen ||
	    (maxxlen - minxlen &&
	     (!stepx || (maxxlen - minxlen) % stepx)))
		fatal("bad value for step size");
}

/*
 * Report the fruits of our labor.
 */
static void
display_results(const char *s,
		struct xfrparms *parmsp,
		struct precise_time *results)
{
	unsigned r;
	struct precise_time *rp;
	unsigned long secs;
	unsigned long nsecs;
	double	t;

	/*
	 * The time value used is the slowest of all the ranks.
	 */
	secs = 0;
	nsecs = 0;
	for (r = 0, rp = results; r < nrank(); r++, rp++) {
		if (debugging || verbose)
			(void )printf("rank %u %s, (elapsed) - %lu.%09lu sec\n",
				      r,
				      s,
				      rp->pt_sec, rp->pt_nsec);
		if (secs <= rp->pt_sec) {
			if ((secs < rp->pt_sec) || (nsecs <= rp->pt_nsec))
				nsecs = rp->pt_nsec;
			secs = rp->pt_sec;
		}
	}

	t = nsecs;
	t /= 1E9;
	t += secs;
	if (t)
		(void )printf("%s%s %lu: elapsed %.3lf, %.3lf MB/s\n",
			      nrank() > 1 ? "aggregate " : "",
			      s,
			      parmsp->xlen,
			      t,
			      ((double )parmsp->nx / t *
			       ((double )parmsp->xlen * nrank()) / 1E6));
	else
		(void )printf("%s%s %lu: elapsed %.3f, <infinite> MB/s\n",
			      nrank() > 1 ? "aggregate " : "",
			      s,
			      parmsp->xlen,
			      t);
}

static void
add2map(const char *s)
{
	size_t	len;
	static size_t map_len = 0;
	void	*p;

	if (debugging)
		msg("DEBUG", "map %u: \"%s\"", map_entry_count, s);

	len = strlen(s);
	p = realloc(map, map_len + len + 1);
	if (p == NULL)
		fatal("can't grow map");
	map = p;
	(void )memcpy(map + map_len, s, len + 1);

	map_offsets =
	    realloc(map_offsets, (map_entry_count + 1) * sizeof(size_t));
	if (map_offsets == NULL)
		fatal("can't gro map offsets");
	map_offsets[map_entry_count++] = map_len;
	map_len += len + 1;
}

static void
get_file_map(char *const path)
{
	FILE	*sd;
	int	err;
	unsigned lincnt;
	unsigned line_too_long;
	unsigned i;
	char	*cp;
	int	c;
	static char line[1025];

	sd = fopen(path, "r");
	if (sd == NULL)
		fatal("%s: %s", path, strerror(errno));
	err = 0;
	lincnt = 1;
	for (;;) {
		/*
		 * Fill the line buffer sans the NL character.
		 */
		line_too_long = 0;
		i = 0;
		cp = line;
		for (;;) {
			c = getc(sd);
			if (c < 0 || c == '\n')
				break;
			if (i >= sizeof(line) - 1) {
				if (!line_too_long) {
					error("%s: map line %u is too long",
					      path,
					      lincnt);
					line_too_long = 1;
					err = 1;
				}
				continue;
			}
			*cp++ = c;
			i++;
		}
		if (c < 0)
			break;				/* EOF */
		*cp = '\0';				/* NUL terminate */

		for (cp = line; *cp != '\0'; cp++)
			if (!(isascii(*cp) && isspace(*cp)))
				break;
		if (*cp == '#')
			continue;			/* skip comment */
		if (cp != line) {
			error("%s: map line %u is malformed", path, lincnt);
			err = 1;
			continue;
		}

		add2map(line);
	}

	(void )fclose(sd);

	if (err)
		all_abort(1);
}
