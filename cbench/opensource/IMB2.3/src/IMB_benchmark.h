/*****************************************************************************
 *                                                                           *
 * Copyright (c) 2003-2004 Intel Corporation.                                *
 * All rights reserved.                                                      *
 *                                                                           *
 *****************************************************************************

This code is covered by the Community Source License (CPL), version
1.0 as published by IBM and reproduced in the file "license.txt" in the
"license" subdirectory. Redistribution in source and binary form, with
or without modification, is permitted ONLY within the regulations
contained in above mentioned license.

Use of the name and trademark "Intel(R) MPI Benchmarks" is allowed ONLY
within the regulations of the "License for Use of "Intel(R) MPI
Benchmarks" Name and Trademark" as reproduced in the file
"use-of-trademark-license.txt" in the "license" subdirectory. 

THE PROGRAM IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT
LIMITATION, ANY WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT,
MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. Each Recipient is
solely responsible for determining the appropriateness of using and
distributing the Program and assumes all risks associated with its
exercise of rights under this Agreement, including but not limited to
the risks and costs of program errors, compliance with applicable
laws, damage to or loss of data, programs or equipment, and
unavailability or interruption of operations.

EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, NEITHER RECIPIENT NOR
ANY CONTRIBUTORS SHALL HAVE ANY LIABILITY FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING
WITHOUT LIMITATION LOST PROFITS), HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OR
DISTRIBUTION OF THE PROGRAM OR THE EXERCISE OF ANY RIGHTS GRANTED
HEREUNDER, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGES. 

EXPORT LAWS: THIS LICENSE ADDS NO RESTRICTIONS TO THE EXPORT LAWS OF
YOUR JURISDICTION. It is licensee's responsibility to comply with any
export regulations applicable in licensee's jurisdiction. Under
CURRENT U.S. export regulations this software is eligible for export
from the U.S. and can be downloaded by or otherwise exported or
reexported worldwide EXCEPT to U.S.  embargoed destinations which
include Cuba, Iraq, Libya, North Korea, Iran, Syria, Sudan,
Afghanistan and any other country to which the U.S. has embargoed
goods and services.

 ***************************************************************************

For more documentation than found here, see

[1] doc/ReadMe_IMB.txt 

[2] Intel (R) MPI Benchmarks
    Users Guide and Methodology Description
    In 
    doc/IMB_ug.pdf
    
 ***************************************************************************/




#ifndef __Bmark_h__
#define __Bmark_h__



/* Classification of benchmarks */

typedef enum { BTYPE_INVALID=-1,
               SingleTransfer,
               ParallelTransfer,
               Collective,
               Sync
             } BTYPES;


typedef struct cmode
          {
          int AGGREGATE ;   /* -1/0/1 for default/NON AGGREGATE/AGGREGATE */
          int NONBLOCKING  ;   /* 0/1 for no/yes */
          BTYPES type;      
          }
        *MODES;

#define X_MODES 2

typedef enum { put, get, no } DIRECTION;

#ifdef MPIIO

typedef enum { nothing=-1,private, explicit, indv_block, indv_cyclic, shared }
               POSITIONING;
#else

typedef int POSITIONING;

#endif

/* Descriptor for benchmarks */

struct Bench
  {
  char*  name;
  char** bench_comments;       /* Comments from header => Comments.h */

  DIRECTION access;

  int reduction;               /* reduction-type y/n */

  int N_Modes;
  struct cmode RUN_MODES[X_MODES];

  void (*Benchmark)(struct comm_info* c_info,int size,
                    int n_sample,MODES RUN_MODE,double* time);
                               /* Pointer to function runnning the benchmark */

  double scale_time, scale_bw; /* Scaling of timings and bandwidth */
  int Ntimes;

  int success;

#ifdef MPIIO
  POSITIONING fpointer;
#endif
  };

#endif
