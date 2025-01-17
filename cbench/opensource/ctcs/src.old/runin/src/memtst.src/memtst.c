#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <malloc.h>
#include "memtst.h"
#include "memory.h"

#include <sys/user.h>


/* The verbose global from memtst_main.c */
extern int verbose;


/* Test Functions */
/* Don't forget to update the #define NUMOFTESTS in memtst.h if you add 
   some! 

   There's a lot of attempts here to make sure that the important cases
   (espec. plus==0) are optimized.  So it looks a little ugly. 

   Many of these tests assume 4 byte integers.  We should have these #ifdefed
   or something for different architectures.   Hey, at least you don't have 
   to go into the nasty part of the code for it anymore... ;) */

/* The original Larry function.  Soon we will add more patterns. */
void larry_write(int *nbuf, int block_size, int plus) {
	unsigned int i;
	if (plus != 0) {
		for(i=0; i < block_size ; ++i) {
			nbuf[i]=i+HEX_AS+plus;
		}
	} else {
		for(i=0; i < block_size ; ++i) {
			nbuf[i]=i+HEX_AS;
		}
	}
}

int larry_check(int *nbuf, int block_size, int *error) {
	unsigned int i;
	for(i=0 ; i < block_size ; ++i) {
		if (nbuf[i] != i+HEX_AS) {
			*error=1;
			return i;
		}
	}
	return 0;
}

/* Different direction for larry's test */
void larry_bkwds_write(int *nbuf, int block_size, int plus) {
	unsigned int i;

	if (plus != 0) {
		for(i=0; i < block_size ; ++i) {
			nbuf[i]=HEX_AS-(i+plus);
		}
	} else {
		for(i=0; i < block_size ; ++i) {
			nbuf[i]=HEX_AS-i;
		}
	}
}

int larry_bkwds_check(int *nbuf, int block_size, int *error) {
	unsigned int i;
	for(i=0 ; i < block_size ; ++i) {
		if (nbuf[i] != HEX_AS-i) {
			*error=1;
			return i;
		}
	}
	return 0;
}

/* Like Larry's pattern, from the top down. */
void larry_top_write(int *nbuf, int block_size, int plus) {
	unsigned int i;
	if (plus != 0) {
		for(i=0; i < block_size ; ++i) {
			nbuf[i]=0 - (i+plus);
		}
	} else {
		for(i=0; i < block_size ; ++i) {
			nbuf[i]=0 - (i+plus);
		}
	}
}

int larry_top_check(int *nbuf, int block_size, int *error) {
	unsigned int i;
	for(i=0 ; i < block_size ; ++i) {
		if (nbuf[i] != 0 - i) {
			*error=1;
			return i;
		}
	}
	return 0;
}

/* Another variation on a theme... */
void larry_bottom_write(int *nbuf, int block_size, int plus) {
	unsigned int i;
	if (plus != 0) {
		for(i=0; i < block_size ; ++i) {
			nbuf[i]=(i+plus);
		}
	} else {
		for(i=0; i < block_size ; ++i) {
			nbuf[i]=i;
		}
	}
}

int larry_bottom_check(int *nbuf, int block_size, int *error) {
	unsigned int i;
	for(i=0 ; i < block_size ; ++i) {
		if (nbuf[i] != i) {
			*error=1;
			return i;
		}
	}
	return 0;
}

/* Jason's function. */
void bitpatts_write(int *nbuf, int block_size, int plus) {
#if SIZEOF_INT == 4
	static unsigned int a[] = { 0xaf05af05, 0xf05af05a, 0x05af05af, 0x5af05af0 };
#endif
#if SIZEOF_INT == 8
	static unsigned int a[] = { 0xaf05af05af05af05, 0xf05af05af05af05a, 0x05af05af05af05af, 0x5af05af05af05af0 } ;
#endif
	unsigned int i;
	if (plus != 0) {
		for(i=0; i < block_size ; ++i) {
			nbuf[i]=a[(i+plus)%4];
		}
	} else {
		for(i=0; i < block_size ; ++i) {
			nbuf[i]=a[i%4];
		}
	}
}

int bitpatts_check(int *nbuf, int block_size, int *error) {
	static unsigned int a[] = { 0xaf05af05, 0xf05af05a, 0x05af05af, 0x5af05af0 };
	unsigned int i;
	for(i=0 ; i < block_size ; ++i)
		if (nbuf[i] != a[i%4]) {
			*error=1;
			return i;
		}
	return 0;
}



/**************/
    



/* Based on configured ceiling (and real memory available) compute the number
   of integers we need in our buffer. */
unsigned long compute_nint(unsigned long free_vm, unsigned long ceiling) {
	unsigned long nint;
	int extra_vm;
	struct memory mem;
		/* Get size of main memory */
	meminfo(&mem);

	/*
	 * Malloc a region the size of main memory , or as large as we
	 * can get.  If the system has enough swap space, this will
	 * let other programs continue to run while we do this test.
	 * Also, it appears to be a large enough region to exercise
	 * most of main memory.  Leave at least free_vm MB of free
	 * virtual memory for other burn-in tests.  If the swap space
	 * is less than free_vm, then this means we decrease the size
	 * of the buffer to leave more free memory.
	 */
	extra_vm = (free_vm * 1024*1024 - mem.total_swap);
	if(extra_vm > 0) {
		nint = (unsigned) ((mem.total_mem - extra_vm)/sizeof(int));
	} else {
		nint = (unsigned) ((mem.total_mem - 4*1024*1024)/sizeof(int));
	}
	if(nint*sizeof(int) > ceiling*1024*1024) {
		nint=(ceiling*1024*1024)/sizeof(int);
	}

	printf("Ceiling: %luK\n", (ceiling*1024));
	printf("Attempting: %dK\n", (int) (nint * sizeof(int)/1024));
	printf("Testing: %lu integers, %luK\n", nint, nint * (long) sizeof(int) /1024);

	return nint;
}


/* Even though we pass an aligned buffer, kmemscan (called from here) still
   needs to know the real alignment for parsing /proc/kcore. */
void display_failure (int *nbuf, int align, int block_size, int offset, 
		      int test_function_write()) {
	/* Window bounds and iterator */
	int low,high,l;
	int *expected;
	char *errorfmt="%18s %18s %18s";
	/* How big of a window do we want? */
	int windowsize=10;
	/* And where (relative to offset) do we wish to start it?  Note
	   that useless values will cause problems (windowstart > 0,
	   windowsize < 0, etc) */
	int windowstart=-3;

	/* First print basic information.  In the unlikely event
	   that we're unable to use kmemscan or it crashes,
	   this guarantees that at least SOMETHING will be shown
	   the user if at all possible. */
	fprintf(stderr, 
		"Error at offset %d of %d, alignment %d:\n", 
		offset, block_size, align);
	fprintf(stderr, "Local process address: %p\n", &nbuf[offset] );

	fprintf(stderr, "Failure Context: \n");
	fprintf(stderr, errorfmt,"offset","expected","got");
	fprintf(stderr, "\n");
	expected=malloc(sizeof(int)*windowsize);

	/* Make sure our viewing window doesn't cross borders... */
	if((low = offset + windowstart) < 0 ) low=0;
	if((high = low + windowsize) > block_size) high=block_size;
	windowsize = high-low;
	if (windowsize <= 0) 
		fprintf(stderr, 
			"WARNING:display_failure called w/invalid block_size");
	
	expected=malloc(sizeof(int)*windowsize);
	test_function_write(expected,windowsize,low);

	for(l=low; l < high ; ++l) {
		fprintf(stderr, "%18d %18x %18x",l,expected[l-low],nbuf[l]);
		if (l == offset) {
			fprintf(stderr, "  *** fail location");
		}
		fprintf(stderr, "\n");
	}
	free(expected);	
	/*kmemscan(nbuf, block_size, offset);*/

	fflush(stdout);
	fflush(stderr);
}




/* generate the table of block sizes bases on a maximum size and a size of
   the table.  There's currently some magic behaviour here when
   the block table size is 0. */
int *generate_block_table(unsigned long nint, int *block_table_size) {
	int *block_table;
	int b;
	/* If the block table size is zero, treat it specially (make a table
	   of one encompassing the entire memory area) */
	if (*block_table_size == 0) {
		block_table=malloc(sizeof(int));
		block_table[0]=nint;
		*block_table_size=1;
		return(block_table);
	}
	/* initialize block size array */
	block_table=malloc(sizeof(int) * *block_table_size);
	for(b=0 ; b < *block_table_size ; ++b) {
		if (b < 9) {
			/* In increments of 64 K */
			block_table[b] = ((b+1) * 64) * 1024 / sizeof(int); 
			if (block_table[b] > nint) block_table[b] = nint;
		} else {
			/* Divide the buffer up into progressively larger
			   chunks */
#if 0
			printf("%d / %d\n",b-8,*block_table_size-9);
#endif
			block_table[b] = (nint * (b-8)) / (*block_table_size-9);
			if (block_table[b] > nint) block_table[b] = nint;  
		}
	}
	return block_table;
}

/* This isn't good.  get_buf can sometimes reduce nint on us. We're
   trying to be user/programmer friendly here, but it may not be
   appropriate.  I'll have to try some things out.  But we should
   eventually pass by value, not reference. */
char *get_buf(unsigned long *nint) {
	unsigned long tryint;
	char *buf;
	tryint = *nint;
	while((buf = (char *)calloc(*nint,sizeof(int))) == NULL) {
		if(verbose) {
			printf("Unable to allocated requested memory.\n");
			printf("Decreasing request size by 5%%.\n");
		}
		*nint = *nint*0.95;
		if(*nint < .7*tryint) {
			fprintf(stderr, "Can't get enough memory to be worthwhile.  Giving up.\n");
			exit(2);
		}
	}
	return buf;
}


/* Takes aligned buffers.  Finds an arbitrary location in memory of a
 * user process (must be passed as an array of ints) and translates
 * it to physical memory location.  All this without kernel support
 * (besides /proc/kcore). Optionally specify a character alignment in
 * case the memory buffer is not byte-aligned. */
void kmemscan (int *nbuf, int block_size, int offset) {
	int kmem_file;
	int d;

	/* window manipulation, iterator, read retval, etc */
	int low, high, foo;
      	int rd;

	/* We will use the fact that Linux does not suballocate pages (does
	   any real OS do that?) to optimize the search by only looking in the
	   parts of the page which matter. */
	int fail_page_offset;
	//int curr_page_offset;

	/* Physical address pointers */
	unsigned long phys_addr=0;
	unsigned long failure;
	unsigned long pages=0;

	/* Don't put weird values here (size < 0, start > 0) */
	int windowsize=10;
	int windowstart=-5;

	long page_size = sysconf (_SC_PAGESIZE); 

	/* Now compute the offset (in chars) of the error from the page
	   boundary. */
	fail_page_offset = ((int) (&nbuf[offset])) % page_size;

	kmem_file = open("/proc/kcore",0);
	if (kmem_file < 0) {
		printf("Unable to open /proc/kcore.  No memory failure scanning possible.\n");
		return;
	}

	/* Set up window */
	if ((low = offset + windowstart) < 0) low=0;
	if ((high = low + windowsize) > block_size) high=block_size;
	foo=low;

	/* fail_page_offset is now the offset of the beginning of the
	 * window.
	 */
	fail_page_offset -= ((offset - low) * sizeof(int));
	if (fail_page_offset < 0) fail_page_offset+=page_size;

	printf("%d %x fail_page_offset\n",fail_page_offset,fail_page_offset);

	fprintf(stderr, "Scanning /proc/kcore.  Fire in the hole!\n");


	/* Start by seeking to the start of the area on the page where
	 * we'st be lookin'
	 */
	lseek(kmem_file,pages*page_size+fail_page_offset,SEEK_SET);
	phys_addr=pages*page_size+fail_page_offset;

	/* We now use lseeks to (hugely) improve the performance of this
	   thing.  Large memory systems were extremely painful before. 
	   Away we go! */
	while( (rd=read(kmem_file, &d, sizeof(int))) != 0) {
		phys_addr += rd;
		if(nbuf[foo] == d) {
			++foo;
		} else {
			foo = low;	
			/* Every time we miss, skip to the next page. */
			++pages;
			lseek(kmem_file,pages*page_size+fail_page_offset,SEEK_SET);
			phys_addr=pages*page_size+fail_page_offset;
			continue;
		}
		/* If foo made it to high, we've found it. */
		if(foo==high) {
			/* Convert to address.  Our physical address is
			   pointing at high, we need it to point at the
			   window center.  */
			failure = phys_addr - 
				sizeof(int) * (windowsize + windowstart);
			fprintf(stderr, "Possible location of memory failure: %p (%dM) on page %d\n",
				(void *) failure,
				(int) (failure/1024/1024),
				(int) (failure/page_size));
			close(kmem_file);
			return;
		} 

	}
	close(kmem_file);
	fprintf(stderr, "The memory failure location could not be determined. This,\nwhile not provably impossible, should never happen under practical\ncircumstances unless there is a bug or the memtst program image is\n corrupt.\n");
}


/* Test a buffer with the alignment/block size given */
/* Return result */
int test_block (char *buf, int align, int block_size, 
		int test_function_write(), int test_function_check()) {
	int offset;
	int *nbuf;
	int error=0;
	
	/* Implement aligns the way Larry intended to, one char (not
	 * int) off at a time.  In Larry's original code, he used an int
	 * pointer for buf without casting it, bad, bad, bad! */
	nbuf = (int *) (&buf[align]);
	if (align != 0) {
		--block_size;  /* avoid hitting outside the buffer.
				  This was a problem in Larry's original code
				  too, but since he was rounding down in
				  block size calculations it didn't come
				  up in practice. It still shouldn't, but.. */
	}

	/* Set up nbuf */
	test_function_write(nbuf, block_size, 0);

	/* fake errors.  Def them in if you need to check memtst's behavior. */
#if 0
	/* test generic error handling */
	nbuf[15681]=0x1234;
#endif

#if 0
	/* test alignment error handling */
	if(align==0) {
		nbuf[12617]=0x2345;
	}
#endif

	/* Verify the result */
	if(verbose) printf("Verifying...");
	fflush(stdout);

	/* test_function_check replaces the old test loop.  Since we've
	   passed this function in (and test_function_write, above), we
	   can now use arbitrary test functions as mentioned at the top
	   of this file. */
	offset=test_function_check(nbuf, block_size, &error);
	if(error != 0) {
		display_failure(nbuf, align, block_size, offset, test_function_write);
		/* clear out the memtest buffer.  Not strictly needed for 
		   normal purposes, but when testing the failure detection it 
		   helps keep from detecting with kmemscan an error from
		   a different process or even a non-running process. */
		for(offset = 0; offset < block_size; offset++) {
			nbuf[offset] = 0;
		}
		return(1);
	}
	if (verbose) printf("Done.\n");
	fflush(stdout);
	return(0);
}

/* return a tests structure with tests initalized, takes pointers to test 
   functions (see test_pattern, above) and descriptions. */
tests *test_setup (int num_tests, void **testfunctions_write, 
		   void **testfunctions_check, char **descriptions ) {
	int i;
	test_pattern **patterns;
	tests *memtests;

	patterns=(test_pattern **) (malloc(sizeof(test_pattern *)*num_tests));

	for (i=0; i<num_tests; ++i) {
		patterns[i]=(test_pattern *) malloc(sizeof(test_pattern));
		patterns[i]->testfunction_write = testfunctions_write[i];
		patterns[i]->testfunction_check = testfunctions_check[i];
		patterns[i]->desc = descriptions[i];
	}

	memtests = (tests *) (malloc(sizeof(tests)));
	memtests->numoftests = num_tests;
	memtests->patterns=patterns;
	return memtests;
}


/* Main function */
/* Drives everything else in the module. */
int memtst(tests *memtests, char *buf, int *block_table, int block_table_size) {
	/* Alignment, block, and test iterators */
	int align, b, t;

	/*
	 * There are 2 things we vary as we walk across memory: the
	 * alignment and the block size.  The alignment varies from 0
	 * to 3.  We write 4 byte words on non-aligned byte boundaries
	 * hoping to detect any errors caused by crossing non-aligned
	 * boundaries.  The block size is the number of bytes that we
	 * write before doing a verifying read.  It varies from 64K to
	 * the entire buffer size.  This exercises cache more with the
	 * small block sizes and main memory more with larger block
	 * sizes.
	 */

	/* The Big Loop */
	/* for each block... */
	for(b=0 ; b < block_table_size ; ++ b) {
	/* for each alignment... */
	for(align = 0; align < sizeof(int) ; align++) {
	/* for each test */
	for (t = 0; t < memtests->numoftests ; ++t) {
		if(verbose) 
			printf("Testing block_size %d (%dK), alignment %d, with %s...", 
			       block_table[b], 
			       block_table[b]*sizeof(int)/1024,
			       align,
			       memtests->patterns[t]->desc);
		fflush(stdout);
		if(test_block(buf,
			      align,
			      block_table[b],
			      memtests->patterns[t]->testfunction_write, 
			      memtests->patterns[t]->testfunction_check)) {
			/* test failed. */
			if ( b < 5 ) {
				fprintf(stderr, "Cache RAM fault likely.\n");
			} else {
				fprintf(stderr, "System RAM fault likely.\n");
			}
			free(buf) ; 
			return(1);
		}
	}
	}
	}

	printf(" OK.\n");
	return (0);
}
