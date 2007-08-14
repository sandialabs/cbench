#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "prctim.h"

/*
 * Get the current value of the clock.
 */
void
get_precise_time(struct precise_time *prctimp)
{
	struct timeval tv;

	if (gettimeofday(&tv, NULL) != 0)
		abort();				/* can't happen */
	prctimp->pt_sec = tv.tv_sec;
	prctimp->pt_nsec = tv.tv_usec * 1000;
}

/*
 * Return the difference of two clock values in the third argument. It
 * does not matter which is the larger of the two.
 */
void
diff_precise_time(struct precise_time *prctim1p,
		  struct precise_time *prctim2p,
		  struct precise_time *prctimrp)
{

	if (prctim1p->pt_sec > prctim2p->pt_sec ||
	    (prctim1p->pt_sec == prctim2p->pt_sec &&
	     prctim1p->pt_nsec > prctim2p->pt_nsec)) {
		struct precise_time *ptp;

		ptp = prctim1p;
		prctim1p = prctim2p;
		prctim2p = ptp;
	}

	if (prctim2p->pt_nsec < prctim1p->pt_nsec) {
		prctim2p->pt_sec--;
		prctim2p->pt_nsec += 1E9;
	}

	prctimrp->pt_sec = prctim2p->pt_sec - prctim1p->pt_sec;
	prctimrp->pt_nsec = prctim2p->pt_nsec - prctim1p->pt_nsec;
}
