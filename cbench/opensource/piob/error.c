#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>

#include "prctim.h"
#include "piob.h"

static char msgbuf[8192];				/* error msg buffer */

static void
outstr(const char *s)
{

	(void )fprintf(stderr, "%s\n", s);
	(void )fflush(stderr);
}

static void
vmsg(const char *q, const char *s, va_list ap)
{

	(void )sprintf(msgbuf, "RANK %d:%s: ", self(), q);
	(void )vsprintf(msgbuf + strlen(msgbuf), s, ap);
	outstr(msgbuf);
}

void
msg(const char *q, const char *s, ...)
{
	va_list	ap;

	va_start(ap, s);
	vmsg(q, s, ap);
	va_end(ap);
}

/*
 * Print some error message.
 */
void
error(const char *s, ...)
{
	va_list	ap;

	va_start(ap, s);
	vmsg("ERROR", s, ap);
	va_end(ap);
}

/*
 * Print fatal error message and exit with non-zero code.
 */
void
fatal(const char *s, ...)
{
	va_list	ap;

	va_start(ap, s);
	vmsg("FATAL", s, ap);
	va_end(ap);

	all_abort(1);

	abort();
}
