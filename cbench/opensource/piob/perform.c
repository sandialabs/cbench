#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "prctim.h"
#include "piob.h"

static const char *file_path = NULL;			/* scratch file name */
static int file_descriptor = -1;			/* scratch fildes */
static struct xfrparms parameters;			/* xfr params */

/*
 * Map system error number into reply codes.
 */
static int
map_reply(int code)
{

	return code == 0 ? 0 : -1;
}

/*
 * Set the parameters in the local process.
 *
 * Return zero on success. Otherwise a positive error code is returned.
 */
void
perform_set_parameters(struct xfrparms *arg, int *result)
{

	parameters.xlen = arg->xlen;
	parameters.nx = arg->nx;
	parameters.step = arg->step;
	*result = map_reply(0);
}

/*
 * Clsoe the currently opened file and free resources associated with it's
 * path.
 *
 * If the open file should be kept it will *not* be unlinked.
 *
 * If forced, it is not an error to have a path recorded without the associated
 * file being open and the reverse.
 *
 * Return zero on success. Otherwise a positive error code is returned.
 */
static int
unset_file(int keep, int forced)
{
	int	err;

	err = 0;
	if (file_descriptor < 0 && !forced) {
		error("no file descriptor to close");
		err = EINVAL;
	}
	if (file_descriptor >= 0 && close(file_descriptor) != 0) {
		if (!err)
			err = errno;
		error("%s: %s", file_path, strerror(err));
	}
	file_descriptor = -1;
	if (!keep) {
		if (file_path == NULL && !forced) {
			error("no file path to unlink");
			if (!err)
				err = EINVAL;
		}
		if (file_path != NULL && unlink(file_path) != 0) {
			if (!err)
				err = errno;
			error("%s: %s", file_path, strerror(err));
		}
	}
	if (file_path != NULL)
		free((char *)file_path);
	file_path = NULL;

	return err;
}

/*
 * Open the given file with the given flags and remember it's name.
 *
 * Return zero on success. Otherwise a positive error code is returned.
 */
void
perform_set_file(struct filargs *arg, int *result)
{
	int	err;
	int	flags;

	if (debugging)
		msg("DEBUG",
		    "Set file \"%s %s-0%o",
		    arg->path,
		    arg->new ? "new" : "old",
		    arg->mode);

	/*
	 * Must invalidate existing resource first!
	 */
	if (file_path != NULL || file_descriptor >= 0) {
		*result = EBUSY;
		return;
	}

	err = 0;
	if (!err) {
		if (file_path != NULL)
			free((char *)file_path);
		file_path = malloc(strlen(arg->path) + 1);
		if (file_path == NULL)
			err = ENOMEM;
		if (!err)
			(void )strcpy((char *)file_path, arg->path);
	}

	flags = 0;
	if (arg->new)
		flags |= O_EXCL|O_CREAT;
	switch (arg->mode) {
	
	case 0:
		flags |= O_RDONLY;
		break;
	case 1:
		flags |= O_WRONLY;
		break;
	case 2:
		flags |= O_RDWR;
		break;
	default:
		fatal("bad mode");
		break;
	}

	if (!err && (file_descriptor = open(file_path, flags, 0666)) < 0) {
		err = errno;
		error("%s (flags 0%o): %s", file_path, flags, strerror(errno));
	}

	if (err && file_descriptor >= 0)
		(void )unset_file(0, 1);

	*result = map_reply(err);
}

/*
 * Close the currently open file and forget it's path name. Unless the
 * file is to be kept, it is unlinked.
 *
 * Return zero on success. Otherwise a positive error code is returned.
 */
void
perform_close_file(int *arg, int *result)
{

	*result = map_reply(unset_file(*arg, 0));
}

/*
 * Perform writes on the currently open file according to the current
 * parameters. The argument time is set to the elapsed time for all the
 * writes.
 *
 * Return zero on success. Otherwise a positive error code is returned.
 */
void
perform_writes(int *arg, struct metric *result)
{
	char	*buf;
	int	err, ioerr;
	struct precise_time start, now;
	unsigned long n;
	ssize_t	cc;
	off_t	incr;
	off_t	initial;
	off_t	off;
	off_t	llen;

	(void )memset(result, 0, sizeof(struct metric));

	if (debugging)
		msg("DEBUG",
		    "begin WRITES, locking=%d",
		    *arg);

	buf = malloc(parameters.xlen);
	if (buf == NULL) {
		err = errno;
		error("IO buffer: %s", strerror(err));
		result->err = map_reply(err);
		return;
	}

	/*
	 * All of this is crafted to minimize the work done in the loop.
	 * We're trying to measure the IO speed after all.
	 */
	cc = 0;
	incr = parameters.step - 1;
	incr *= parameters.xlen;
	initial = incr ? self() * parameters.xlen : 0;
	ioerr = err = 0;
	get_precise_time(&start);
	if ((off = lseek(file_descriptor, initial, SEEK_SET)) < 0) {
		err = errno;
		error("%s (1st lseek): %s", file_path, strerror(err));
	}
	if (debugging)
		msg("DEBUG",
		    "pos now %ld, rec # %lu",
		    off,
		    initial / parameters.xlen);
	if (!err)
		for (n = 0; n < parameters.nx; n++) {
			llen = parameters.xlen;
			if (*arg &&
			    lockf(file_descriptor, F_LOCK, llen) != 0) {
				err = errno;
				error("%s (lock): %s",
				      file_path,
				      strerror(err));
				break;
			}
			cc = write(file_descriptor, buf, parameters.xlen);
			llen =- cc;
			if (cc != (ssize_t )parameters.xlen) {
				ioerr = errno;
				if (cc < 0) {
					error("%s (write): %s",
					      file_path,
					      strerror(errno));
					llen = -parameters.xlen;
				} else {
					error("%s: short write", file_path);
					ioerr = EIO;	/* fake it */
				}
			}
			if (*arg) {
				if (lockf(file_descriptor,
					  F_ULOCK,
					  -llen) != 0) {
					err = errno;
					error("%s (unlock): %s",
					      file_path,
					      strerror(err));
					break;
				}
				if (llen >= 0 && llen != parameters.xlen &&
				    lockf(file_descriptor,
					  F_ULOCK,
					  parameters.xlen - llen) != 0) {
					err = errno;
					error("%s (unlock): %s",
					      file_path,
					      strerror(err));
					break;
				}
			}
			if (*arg &&
			    lockf(file_descriptor, F_ULOCK, -llen) != 0) {
				err = errno;
				error("%s (unlock): %s",
				      file_path,
				      strerror(err));
				break;
			}
			if (ioerr) {
				err = ioerr;
				break;
			}
			off = lseek(file_descriptor, incr, SEEK_CUR);
			if (off < 0) {
				err = errno;
				error("%s (lseek): %s",
				      file_path,
				      strerror(err));
				break;
			}
			if (debugging)
				msg("DEBUG",
				    "pos now %ld rec # %ld",
				    off,
				    off / parameters.xlen);
		}
	if (!err && fsync(file_descriptor)) {
		err = errno;
		error("%s (fsync): %s", file_path, strerror(err));
	}
	get_precise_time(&now);

	diff_precise_time(&now, &start, &result->elapsed);

	free(buf);
	result->err = map_reply(err != 0 ? err : 0);
}

/*
 * Perform reads on the currently open file according to the current
 * parameters. The argument time is set to the elapsed time for all the
 * writes.
 *
 * Return zero on success. Otherwise a positive error code is returned.
 */
void
perform_reads(int *arg IS_UNUSED, struct metric *result)
{
	char	*buf;
	int	err, ioerr;
	struct precise_time start, now;
	unsigned long n;
	ssize_t	cc;
	off_t	incr;
	off_t	initial;
	off_t	off;
	off_t	llen;

	(void )memset(result, 0, sizeof(struct metric));

	if (debugging)
		msg("DEBUG",
		    "begin READS, locking=%d",
		    *arg);

	buf = malloc(parameters.xlen);
	if (buf == NULL) {
		err = errno;
		error("IO buffer: %s", strerror(err));
		result->err = map_reply(err);
		return;
	}

	/*
	 * All of this is crafted to minimize the work done in the loop.
	 * We're trying to measure the IO speed after all.
	 */
	cc = 0;
	incr = parameters.step - 1;
	incr *= parameters.xlen;
	initial = incr ? self() * parameters.xlen : 0;
	ioerr = err = 0;
	get_precise_time(&start);
	if ((off = lseek(file_descriptor, initial, SEEK_SET)) < 0) {
		err = errno;
		error("%s (1st lseek): %s", file_path, strerror(err));
	}
	if (debugging)
		msg("DEBUG",
		    "pos now %ld, rec # %lu",
		    off,
		    initial / parameters.xlen);
	if (!err)
		for (n = 0; n < parameters.nx; n++) {
			llen = parameters.xlen;
			cc = read(file_descriptor, buf, parameters.xlen);
			llen =- cc;
			if (cc != (ssize_t )parameters.xlen) {
				ioerr = errno;
				if (cc < 0) {
					error("%s (read): %s",
					      file_path,
					      strerror(err));
					llen = -parameters.xlen;
				} else {
					error("%s: short read", file_path);
					ioerr = EIO;	/* fake it */
				}
			}
			if (ioerr) {
				err = ioerr;
				break;
			}
			off = lseek(file_descriptor, incr, SEEK_CUR);
			if (off < 0) {
				err = errno;
				error("%s (lseek): %s",
				      file_path,
				      strerror(err));
				break;
			}
			if (debugging)
				msg("DEBUG",
				    "pos now %ld rec # %ld",
				    off,
				    off / parameters.xlen);
		}
	get_precise_time(&now);

	diff_precise_time(&now, &start, &result->elapsed);

	free(buf);
	result->err = map_reply(err != 0 ? err : 0);
}

#ifdef OLD_BAD_STUFF
/*
 * Perform reads on the currently open file according to the current
 * parameters. The argument time is set to the elapsed time for all the
 * reads.
 *
 * Return zero on success. Otherwise a positive error code is returned.
 */
void
perform_reads(int *arg IS_UNUSED, struct metric *result)
{
	char	*buf;
	int	err;
	struct precise_time start, now;
	unsigned long n;
	ssize_t	cc;
	off_t	incr;
	off_t	initial;
	off_t	off;

	(void )memset(result, 0, sizeof(struct metric));

	buf = malloc(parameters.xlen);
	if (buf == NULL) {
		err = errno;
		error("IO buffer: %s", strerror(err));
		result->err = map_reply(err);
		return;
	}

	/*
	 * All of this is crafted to minimize the work done in the loop.
	 * We're trying to measure the IO speed after all.
	 */
	cc = 0;
	incr = parameters.step - 1;
	incr *= parameters.xlen;
	initial = incr ? self() * parameters.xlen : 0;
	err = 0;
	get_precise_time(&start);
	if ((off = lseek(file_descriptor, initial, SEEK_SET)) < 0) {
		err = errno;
		error("%s: %s", file_path, strerror(err));
	}
	if (debugging)
		msg("DEBUG",
		    "pos now %ld, rec # %lu",
		    off,
		    initial / parameters.xlen);
	if (!err)
		for (n = 0; n < parameters.nx; n++) {
			cc = read(file_descriptor, buf, parameters.xlen);
			if (cc != (ssize_t )parameters.xlen) {
				err = errno;
				if (cc < 0)
					error("%s: %s",
					      file_path,
					      strerror(err));
				else {
					error("%s: short read", file_path);
					err = EIO;	/* fake it */
				}
				break;
			}
			off = lseek(file_descriptor, incr, SEEK_CUR);
			if (off < 0) {
				err = errno;
				error("%s: %s", file_path, strerror(err));
				break;
			}
			if (debugging)
				msg("DEBUG",
				    "pos now %ld rec # %ld",
				    off,
				    off / parameters.xlen);
		}
	get_precise_time(&now);

	diff_precise_time(&now, &start, &result->elapsed);

	free(buf);
	result->err = map_reply(err != 0 ? err : 0);
}
#endif

/*
 * Force current file to close and forget it's path. The file is unlinked.
 *
 * Return zero on success. Otherwise a positive error code is returned.
 */
void
perform_terminate(void *arg IS_UNUSED, int *result)
{

	unset_file(0, 1);
	run = 0;
	*result = map_reply(0);
}

#ifdef INVALIDATE_CACHED_DATA
void
perform_invalidate(void *arg IS_UNUSED, int *result)
{

	error("invalidate: %s", strerror(ENOTSUP));
	*result = map_reply(ENOTSUP);
}
#endif /* defined(INVALIDATE_CACHED_DATA) */
