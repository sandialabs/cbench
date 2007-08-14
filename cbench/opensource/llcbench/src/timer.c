#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include "llcbench.h"

static double t1, t2;

void timer_start(void)
{
#if defined(USE_GETTIMEOFDAY)
  {
    struct timeval ts;
    gettimeofday(&ts, (struct timezone*)0);
    t1 = (double)ts.tv_sec*1000000000.0 + (double)ts.tv_usec*1000.0; 
  }
#else
  {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME,&ts);
    t1 = (double)ts.tv_sec*1000000000.0 + (double)ts.tv_nsec;
  }
#endif
  DBG(printf("START %f\n",t1))
}

void timer_stop(void)
{
#if defined(USE_GETTIMEOFDAY)
  {
    struct timeval ts;
    gettimeofday(&ts, (struct timezone*)0);
    t2 = (double)ts.tv_sec*1000000000.0 + (double)ts.tv_usec*1000.0; 
  }
#else
  {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME,&ts);
    t2 = (double)ts.tv_sec*1000000000.0 + (double)ts.tv_nsec;
  }
#endif
  DBG(printf("STOP %f\n",t2))
}

double timer_elapsed(void)
{
  if (t2-t1 <= 0.0)
    {
      fprintf(stderr,"Warning! The timer is not precise enough. Consider increasing\nthe iteration count or changing the timer in timer.c\n");
      return(0.0);
    }
  return((t2-t1)/1000.0);
}
