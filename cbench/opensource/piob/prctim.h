/*
 * Common format for all clocks to return.
 */
struct precise_time {
	unsigned long pt_sec;				/* seconds */
	unsigned long pt_nsec;				/* nano-seconds */
};

extern void get_precise_time(struct precise_time *);
extern void diff_precise_time(struct precise_time *,
			      struct precise_time *,
			      struct precise_time *);
