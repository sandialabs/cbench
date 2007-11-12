Intel® MPI Benchmarks

Version 3.0
Release Notes

====================================================================

Main changes vs. IMB_2.3:

- Benchmark "Alltoallv" added
- Flag -h[elp] added for help
- All except 2 makefiles erased
- Better argument line error handling

====================================================================

This document contains the description of the software package
(installation, running, header files and data structures, interfaces
of all functions in IMB). 

For a documentation of the methodologies behind it, see the reference

[1] doc/IMB_ug.pdf


Overview
========

I.   Installing and running IMB
II.  Header files, struct data types
III. All interfaces and brief documentation

I. Installing and running IMB
=============================

I.1 Directory
-------------

After unpacking, the directory contains

ReadMe_first

and 4 subdirectories

./doc  (this file; IMB.pdf, the methodology description)
./src  (program source- and Make-files)
./license (license agreement text)
./versions_news (version history and news)

>>>>
Please read the license agreements first:
  - license.txt specifies the source code license granted to you
  - use-of-trademark-license.txt specifies the license for using the 
    name and trademark "Intel® MPI Benchmarks"
<<<<


I.2 Installation and quick start
--------------------------------

(please read [1] for more extensive explanations).

2 Makefiles are provided:

make_ict   (for Intel Cluster Tools usage)
make_mpich (for mpich; has to be edited)

, invoked by

make -f make_ict <target>
make -f make_mpich <target>

(attention: in contrast to IMB_2.3, these are full makefiles
and don't need to included)

In their header, variables are set:

Mandatory:

CC          = mpicc (e.g.)
CLINKER     = ${CC} (e.g.)

Optional:

MPI_INCLUDE =
LIB_PATH    =
LIBS        = 
OPTFLAGS    = 
LDFLAGS     =
CPPFLAGS    = 

These variables are then exported to the main part of the Makefile,
Makefile.base.

In make_mpich, the root of the installation must be set:

MPI_HOME=/opt/mpich2-1.0.3-icc.icpc-ifort/ch3_ssm


In the end, compilation will follow the rule

	$(CC) $(MPI_INCLUDE) $(CPPFLAGS) $(OPTFLAGS) -c $*.c

and linkage is done by

	$(CLINKER) $(LDFLAGS) -o <exe> <objects>  $(LIB_PATH) $(LIBS)

The only CPPFLAGS setting currently provided is "-DCHECK"; when activated,
IMB checks contents of message passing buffers, as far as possible. Should
be used for correctness check of an implementation only, not for 
performance measurements.

make -f <choice> IMB-<case>, where case is

"MPI1", "EXT" or "MPIIO"


I.3 Running and run time flags
------------------------------

IMB-<case>  [-h{elp}]
            [-npmin  <NPmin>]
            [-multi  <MultiMode>]
            [-msglen <Lengths_file>]
            [-map    <PxQ>]
            [-input  <filename>]
            [benchmark1 [,benchmark2 [,...]]]

where 

- case is one of MPI1, EXT, IO

- h ( or help) just provides basic help 
  (if active, all other arguments are ignored)

- NPmin is the minimum number of processes to run on
  (then if IMB is started on NP processes, the process numbers 
   NPmin, 2*NPmin, ... ,2^k * NPmin < NP, NP are used)
   >>>
   to run on just NP processes, run IMB on NP and select -npmin NP
   <<<
  Default: NPmin=2

- P,Q are integer numbers with P*Q <= NP
  Enter PxQ with the 2 numbers separated by letter "x" and no blanc
  The basic communicator is set up as P by Q process grid

  If, e.g., one runs on N nodes of X processors each, and inserts
  P=X, Q=N, then the numbering of processes is \"inter node first\".
  Running PingPong with P=X, Q=2 would measure inter-node performance
  (assuming MPI default would apply 'normal' mapping, i.e. fill nodes
  first priority) 

  Default: Q=1

- MultiMode is 0 or 1

  if -multi is selected, running the N process version of a benchmark
  on NP overall, means running on (NP/N) simultaneous groups of N each.

  MultiMode only controls default (0) or extensive (1) output charts.
  0: Only lowest performance groups is output
  1: All groups are output

  Default: multi off

- Lengths_file is an ASCII file, containing any set of nonnegative
  message lengths, 1 per line

  Default: no Lengths_file, lengths defined by settings.h, settings_io.h
  
- filename is any text file containing, line by line, benchmark names.
  Facilitates running particular benchmarks as compared to using the
  command line.

  Default: no input file exists
  
- benchmarkX is (in arbitrary lower/upper case spelling)

for case==MPI-1 one of

PingPong
PingPing
Sendrecv
Exchange
Bcast
Allgather
Allgatherv
Alltoall
Alltoallv
Reduce
Reduce_scatter
Allreduce
Barrier

for case==EXT one of

Window
Unidir_Put
Unidir_Get
Bidir_Get
Bidir_Put
Accumulate

for case==MPIIO one of

S_Write_indv
S_Read_indv
S_Write_expl
S_Read_expl
P_Write_indv
P_Read_indv
P_Write_expl
P_Read_expl
P_Write_shared
P_Read_shared
P_Write_priv
P_Read_priv
C_Write_indv
C_Read_indv
C_Write_expl
C_Read_expl
C_Write_shared
C_Read_shared


IMB will run the benchmarks corresponding to well defined rules [1]. The
run settings (message lengths, in particular) are fixed in the header files 
(see next section) 

settings.h (for MPI1, EXT cases) and settings_io.h (for MPIIO).

These should not normally be changed to achieve unified rules. But, they
might be modified when special cases need to be looked at.





II.  Header files, struct data types
====================================

THe following header files belong the the code:

IMB_settings.h
IMB_settings_io.h

IMB_prototypes.h

IMB_benchmark.h
IMB_bnames_ext.h
IMB_bnames_io.h
IMB_bnames_mpi1.h

IMB_comm_info.h

IMB_declare.h
IMB_comments.h
IMB_appl_errors.h
IMB_err_check.h

All header files contain inline documentation, so here only brief
hints are given.

II.1 IMB_settings.h / IMB_settings_io.h
-------------------------------------------
These files fix the run mode of IMB, in particular the message lengths
each benchmark will use. Normally, these should not be changed.


All other headers are IMB internal and must not normally be changed. 
Only for detailed understanding of the code, it is necessary to look
at these files.

II.2. IMB_prototypes.h
------------------------
Collection of all prototypes use in IMB

II.3. IMB_benchmark.h
------------------------
IMB sets up a linked list of benchmarks requested by the user. The description
of a benchmark is collected in a "struct Bench" data structure which is 
defined here. In particular contains a benchmark structure the 'modes' 
(sub-structure 'MODES') of a benchmark which says whether the benchmark is

- single/parallel/collective transfer or synchronisation
- aggregate/non aggregate (only EXT and MIPIO)
- blocking/nonblocking

(see the manual).

This structure "MODES" is also used for the => calling sequences of the benchmarks.

II.4. IMB_bnames_<case>.h
-------------------------

Internal string lists of benchmark names

II.5. IMB_comm_info.h
---------------------

Collection of all (run time dependent) data describing the MPI environment of
a calling process (communicators, process ids etc).

II.6. IMB_declare.h
---------------------
Declaration of global variables and preprocessor macros.

II.7. IMB_comments.h
---------------------
(Currently empty) list of comments attached to each benchmark

II.8. IMB_appl_errors.h, IMB_err_check.h
----------------------------------------
Definition of internal error codes and callback functions for error handlers.





III.  Interfaces with brief documentation
=========================================

The code consists of the following (37) C modules, which in
turn contain several functions eventually. All intefaces will be
listed and documented below.

IMB_allgather.c
IMB_allgatherv.c
IMB_allreduce.c
IMB_alltoall.c
IMB_barrier.c
IMB_bcast.c
IMB_benchlist.c
IMB.c
IMB_chk_diff.c
IMB_cpu_exploit.c
IMB_declare.c
IMB_err_handler.c
IMB_exchange.c
IMB_g_info.c
IMB_init.c
IMB_init_file.c
IMB_init_transfer.c
IMB_mem_manager.c
IMB_ones_accu.c
IMB_ones_bidir.c
IMB_ones_unidir.c
IMB_open_close.c
IMB_output.c
IMB_parse_name_ext.c
IMB_parse_name_io.c
IMB_parse_name_mpi1.c
IMB_pingping.c
IMB_pingpong.c
IMB_read.c
IMB_reduce.c
IMB_reduce_scatter.c
IMB_sendrecv.c
IMB_strgs.c
IMB_user_set_info.c
IMB_warm_up.c
IMB_window.c
IMB_write.c




III.1  MPI-1 message passing benchmarks 
----------------------------------------

All interfaces for MPI-1 benchmarks have the following form

/*

                      
                      MPI-1 benchmark kernel
                      


Input variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      

-size                 (type int)                      
                      Basic message size in bytes

-n_sample             (type int)                      
                      Number of repetitions (for timing accuracy)

-RUN_MODE             (type MODES)                      
                      (only MPI-2 case: see II.)


Output variables: 

-time                 (type double*)                      
                      Timing result per sample


*/
**********************************************************************
File: IMB_allgather.c
**********************************************************************

Implemented function: 

IMB_allgather;




======================================================================
void IMB_allgather(struct comm_info* c_info, int size, int n_sample, 
                   MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-1 benchmark kernel
                      Benchmarks MPI_Allgather
                      

                      (see above for general interface description
                       of MPI-1 benchmarks)

*/


**********************************************************************
File: IMB_allgatherv.c
**********************************************************************

Implemented function: 

IMB_allgatherv;




======================================================================
void IMB_allgatherv(struct comm_info* c_info, int size, int n_sample, 
                    MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-1 benchmark kernel
                      Benchmarks MPI_Allgatherv
                      

                      (see above for general interface description
                       of MPI-1 benchmarks)

*/


**********************************************************************
File: IMB_allreduce.c
**********************************************************************

Implemented function: 

IMB_allreduce;




======================================================================
void IMB_allreduce(struct comm_info* c_info, int size, int n_sample, 
                   MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-1 benchmark kernel
                      Benchmarks MPI_Allreduce
                      

                      (see above for general interface description
                       of MPI-1 benchmarks)

*/


**********************************************************************
File: IMB_alltoall.c
**********************************************************************

Implemented function: 

IMB_alltoall;




======================================================================
void IMB_alltoall(struct comm_info* c_info, int size, int n_sample, 
                  MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-1 benchmark kernel
                      Benchmarks MPI_Alltoall
                      

                      (see above for general interface description
                       of MPI-1 benchmarks)

*/


**********************************************************************
File: IMB_barrier.c
**********************************************************************

Implemented function: 

IMB_barrier;




======================================================================
void IMB_barrier(struct comm_info* c_info, int size, int n_sample, 
                 MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-1 benchmark kernel
                      Benchmarks MPI_Barrier
                      

                      (see above for general interface description
                       of MPI-1 benchmarks)

*/


**********************************************************************
File: IMB_bcast.c
**********************************************************************

Implemented function: 

IMB_bcast;




======================================================================
void IMB_bcast(struct comm_info* c_info, int size, int n_sample, 
               MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-1 benchmark kernel
                      Benchmarks MPI_Bcast
                      

                      (see above for general interface description
                       of MPI-1 benchmarks)

*/


**********************************************************************
File: IMB_exchange.c
**********************************************************************

Implemented function: 

IMB_exchange;




======================================================================
void IMB_exchange(struct comm_info* c_info, int size, int n_sample, 
                  MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-1 benchmark kernel
                      Chainwise exchange; MPI_Isend (left+right) + MPI_Recv (right+left)
                      

                      (see above for general interface description
                       of MPI-1 benchmarks)

*/


**********************************************************************
File: IMB_pingping.c
**********************************************************************

Implemented function: 

IMB_pingping;




======================================================================
void IMB_pingping(struct comm_info* c_info, int size, int n_sample, 
                  MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-1 benchmark kernel
                      2 process exchange; MPI_Isend + MPI_Recv 
                      

                      (see above for general interface description
                       of MPI-1 benchmarks)

*/


**********************************************************************
File: IMB_pingpong.c
**********************************************************************

Implemented function: 

IMB_pingpong;




======================================================================
void IMB_pingpong(struct comm_info* c_info, int size, int n_sample, 
                  MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-1 benchmark kernel
                      2 process MPI_Send + MPI_Recv  pair
                      

                      (see above for general interface description
                       of MPI-1 benchmarks)

*/


**********************************************************************
File: IMB_reduce.c
**********************************************************************

Implemented function: 

IMB_reduce;




======================================================================
void IMB_reduce(struct comm_info* c_info, int size, int n_sample, 
                MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-1 benchmark kernel
                      Benchmarks MPI_Reduce
                      

                      (see above for general interface description
                       of MPI-1 benchmarks)

*/


**********************************************************************
File: IMB_reduce_scatter.c
**********************************************************************

Implemented function: 

IMB_reduce_scatter;




======================================================================
void IMB_reduce_scatter(struct comm_info* c_info, int size, int n_sample, 
                        MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-1 benchmark kernel
                      Benchmarks MPI_Reduce_scatter
                      

                      (see above for general interface description
                       of MPI-1 benchmarks)

*/


**********************************************************************
File: IMB_sendrecv.c
**********************************************************************

Implemented function: 

IMB_sendrecv;




======================================================================
void IMB_sendrecv(struct comm_info* c_info, int size, int n_sample, 
                  MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-1 benchmark kernel
                      Benchmarks MPI_Sendrecv
                      

                      (see above for general interface description
                       of MPI-1 benchmarks)

*/


III.2  MPI-2 onesided communications benchmarks 
------------------------------------------------

All interfaces for MPI-2 onesided communications benchmarks have the following form

/*

                      
                      MPI-2 benchmark kernel
                      


Input variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      

-size                 (type int)                      
                      Basic message size in bytes

-n_sample             (type int)                      
                      Number of repetitions (for timing accuracy)

-RUN_MODE             (type MODES)                      
                      Mode (aggregate/non aggregate; blocking/nonblocking);
                      see "IMB_benchmark.h" for definition


Output variables: 

-time                 (type double*)                      
                      Timing result per sample


*/
**********************************************************************
File: IMB_ones_accu.c
**********************************************************************

Implemented function: 

IMB_accumulate;




======================================================================
void IMB_accumulate (struct comm_info* c_info, int size, int n_sample, 
                     MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-2 benchmark kernel
                      Benchmarks MPI_Accumulate
                      

                      (see above for general interface description
                       of MPI-2 onesided communications benchmarks)

*/


**********************************************************************
File: IMB_ones_bidir.c
**********************************************************************

Implemented functions: 

IMB_bidir_get;
IMB_bidir_put;




======================================================================
void IMB_bidir_get(struct comm_info* c_info, int size, int n_sample, 
                   MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-2 benchmark kernel
                      Driver for aggregate / non agg. bidirectional MPI_Get benchmarks
                      

                      (see above for general interface description
                       of MPI-2 onesided communications benchmarks)

*/




======================================================================
void IMB_bidir_put(struct comm_info* c_info, int size, int n_sample, 
                   MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-2 benchmark kernel
                      Driver for aggregate / non agg. bidirectional MPI_Put benchmarks
                      

                      (see above for general interface description
                       of MPI-2 onesided communications benchmarks)

*/


**********************************************************************
File: IMB_ones_unidir.c
**********************************************************************

Implemented functions: 

IMB_unidir_put;
IMB_unidir_get;
IMB_ones_get;
IMB_ones_mget;
IMB_ones_put;
IMB_ones_mput;




======================================================================
void IMB_unidir_put (struct comm_info* c_info, int size, int n_sample, 
                     MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-2 benchmark kernel
                      Driver for aggregate / non agg. unidirectional MPI_Put benchmarks
                      

                      (see above for general interface description
                       of MPI-2 onesided communications benchmarks)

*/




======================================================================
void IMB_unidir_get (struct comm_info* c_info, int size, int n_sample, 
                     MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-2 benchmark kernel
                      Driver for aggregate / non agg. unidirectional MPI_Get benchmarks
                      

                      (see above for general interface description
                       of MPI-2 onesided communications benchmarks)

*/




======================================================================
void IMB_ones_get(struct comm_info* c_info, int s_num, int dest, 
                  int r_num, int sender, int size, 
                  int n_sample, double* time)
======================================================================
/*

                      
                      Non aggregate MPI_Get + MPI_Win_fence
                      


Input variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      

-s_num                (type int)                      
                      #bytes to put if relevant for calling process 
                      

-dest                 (type int)                      
                      destination rank
                      

-r_num                (type int)                      
                      #bytes to get if relevant for calling process 
                      

-sender               (type int)                      
                      logical flag: 1/0 for 'local process puts/gets'
                      

-size                 (type int)                      
                      Basic message size in bytes
                      

-n_sample             (type int)                      
                      Number of repetitions (for timing accuracy)
                      

-RUN_MODE             (type MODES)                      
                      Mode (aggregate/non aggregate; blocking/nonblocking);
                      see "IMB_benchmark.h" for definition
                      


Output variables: 

-time                 (type double*)                      
                      Timing result per sample
                      


*/




======================================================================
void IMB_ones_mget(struct comm_info* c_info, int s_num, int dest, 
                   int r_num, int sender, int size, 
                   int n_sample, double* time)
======================================================================
/*

                      
                      Aggregate MPI_Get + MPI_Win_fence
                      


Input variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      

-s_num                (type int)                      
                      #bytes to put if relevant for calling process 
                      

-dest                 (type int)                      
                      destination rank
                      

-r_num                (type int)                      
                      #bytes to get if relevant for calling process 
                      

-sender               (type int)                      
                      logical flag: 1/0 for 'local process puts/gets'
                      

-size                 (type int)                      
                      Basic message size in bytes
                      

-n_sample             (type int)                      
                      Number of repetitions (for timing accuracy)
                      

-RUN_MODE             (type MODES)                      
                      Mode (aggregate/non aggregate; blocking/nonblocking);
                      see "IMB_benchmark.h" for definition
                      


Output variables: 

-time                 (type double*)                      
                      Timing result per sample
                      


*/




======================================================================
void IMB_ones_put(struct comm_info* c_info, int s_num, int dest, 
                  int r_num, int sender, int size, 
                  int n_sample, double* time)
======================================================================
/*

                      
                      Non aggregate MPI_Put + MPI_Win_fence
                      


Input variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      

-s_num                (type int)                      
                      #bytes to put if relevant for calling process 
                      

-dest                 (type int)                      
                      destination rank
                      

-r_num                (type int)                      
                      #bytes to get if relevant for calling process 
                      

-sender               (type int)                      
                      logical flag: 1/0 for 'local process puts/gets'
                      

-size                 (type int)                      
                      Basic message size in bytes
                      

-n_sample             (type int)                      
                      Number of repetitions (for timing accuracy)
                      

-RUN_MODE             (type MODES)                      
                      Mode (aggregate/non aggregate; blocking/nonblocking);
                      see "IMB_benchmark.h" for definition
                      


Output variables: 

-time                 (type double*)                      
                      Timing result per sample
                      


*/




======================================================================
void IMB_ones_mput(struct comm_info* c_info, int s_num, int dest, 
                   int r_num, int sender, int size, 
                   int n_sample, double* time)
======================================================================
/*

                      
                      Aggregate MPI_Put + MPI_Win_fence
                      


Input variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      

-s_num                (type int)                      
                      #bytes to put if relevant for calling process 
                      

-dest                 (type int)                      
                      destination rank
                      

-r_num                (type int)                      
                      #bytes to get if relevant for calling process 
                      

-sender               (type int)                      
                      logical flag: 1/0 for 'local process puts/gets'
                      

-size                 (type int)                      
                      Basic message size in bytes
                      

-n_sample             (type int)                      
                      Number of repetitions (for timing accuracy)
                      

-RUN_MODE             (type MODES)                      
                      Mode (aggregate/non aggregate; blocking/nonblocking);
                      see "IMB_benchmark.h" for definition
                      


Output variables: 

-time                 (type double*)                      
                      Timing result per sample
                      


*/


**********************************************************************
File: IMB_window.c
**********************************************************************

Implemented function: 

IMB_window;




======================================================================
void IMB_window(struct comm_info* c_info, int size, int n_sample, 
                MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-2 benchmark kernel
                      MPI_Win_create + MPI_Win_fence + MPI_Win_free
                      

                      (see above for general interface description
                       of MPI-2 onesided communications benchmarks)

*/


III.3  MPI-IO benchmarks 
-------------------------

All interfaces for MPI-IO benchmarks have the following form

/*

                      
                      MPI-IO benchmark kernel
                      


Input variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      

-size                 (type int)                      
                      Basic message size in bytes

-n_sample             (type int)                      
                      Number of repetitions (for timing accuracy)

-RUN_MODE             (type MODES)                      
                      Mode (aggregate/non aggregate; blocking/nonblocking);
                      see "IMB_benchmark.h" for definition


Output variables: 

-time                 (type double*)                      
                      Timing result per sample


*/
**********************************************************************
File: IMB_open_close.c
**********************************************************************

Implemented function: 

IMB_open_close;




======================================================================
void IMB_open_close(struct comm_info* c_info, int size, int n_sample, 
                    MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-IO benchmark kernel
                      MPI_File_open + MPI_File_close
                      

                      (see above for general interface description
                       of MPI-IO benchmarks)

*/


**********************************************************************
File: IMB_read.c
**********************************************************************

Implemented functions: 

IMB_read_shared;
IMB_read_indv;
IMB_read_expl;
IMB_read_ij;
IMB_iread_ij;




======================================================================
void IMB_read_shared(struct comm_info* c_info, int size, int n_sample, 
                     MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-IO benchmark kernel
                      Driver for read benchmarks with shared file pointers
                      

                      (see above for general interface description
                       of MPI-IO benchmarks)

*/




======================================================================
void IMB_read_indv(struct comm_info* c_info, int size, int n_sample, 
                   MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-IO benchmark kernel
                      Driver for read benchmarks with individual file pointers
                      

                      (see above for general interface description
                       of MPI-IO benchmarks)

*/




======================================================================
void IMB_read_expl(struct comm_info* c_info, int size, int n_sample, 
                   MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-IO benchmark kernel
                      Driver for read benchmarks with explicit offsets
                      

                      (see above for general interface description
                       of MPI-IO benchmarks)

*/




======================================================================
void IMB_read_ij(struct comm_info* c_info, int size, POSITIONING pos, 
                 BTYPES type, int i_sample, int j_sample, 
                 int time_inner, double* time)
======================================================================
/*

                      
                      Calls the proper read functions, blocking case
                      


Input variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      

-size                 (type int)                      
                      portion size in bytes
                      

-pos                  (type POSITIONING)                      
                      (see IMB_benchmark.h for definition of this enum type)
                      descriptor for the file positioning
                      

-type                 (type BTYPES)                      
                      (see IMB_benchmark.h for definition of this enum type)
                      descriptor for the file access synchronism
                      

-i_sample,j_sample    (type int)                      
                      aggregate case:     
                      i_sample=1, j_sample=n_sample (set by driving function)
                      non aggregate case: 
                      i_sample=n_sample, j_sample=1 (set by driving function)
                      Benchmark logistics then:
                        for( i=0 .. i_sample-1 )
                           for( j=0 .. j_sample-1 )
                                   input ...
                                   Synchronize (!)
                      

-time_inner           (type int)                      
                      logical flag: should timing be issued for the innermost loop 
                      (and then averaged by outermost count), or for outermost loop
                      


Output variables: 

-time                 (type double*)                      
                      Timing result per sample
                      


*/




======================================================================
void IMB_iread_ij(struct comm_info* c_info, int size, POSITIONING pos, 
                  BTYPES type, int i_sample, int j_sample, 
                  int time_inner, int do_ovrlp, double* time)
======================================================================
/*

                      
                      Calls the proper read functions, non blocking case
                      
                      (See IMB_read_ij for documentation of calling sequence)
                      


*/


**********************************************************************
File: IMB_write.c
**********************************************************************

Implemented functions: 

IMB_write_shared;
IMB_write_indv;
IMB_write_expl;
IMB_write_ij;
IMB_iwrite_ij;




======================================================================
void IMB_write_shared(struct comm_info* c_info, int size, int n_sample, 
                      MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-IO benchmark kernel
                      Driver for write benchmarks with shared file pointers
                      

                      (see above for general interface description
                       of MPI-IO benchmarks)

*/




======================================================================
void IMB_write_indv(struct comm_info* c_info, int size, int n_sample, 
                    MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-IO benchmark kernel
                      Driver for write benchmarks with individual file pointers
                      

                      (see above for general interface description
                       of MPI-IO benchmarks)

*/




======================================================================
void IMB_write_expl(struct comm_info* c_info, int size, int n_sample, 
                    MODES RUN_MODE, double* time)
======================================================================
/*

                      
                      MPI-IO benchmark kernel
                      Driver for write benchmarks with explicit offsets
                      

                      (see above for general interface description
                       of MPI-IO benchmarks)

*/




======================================================================
void IMB_write_ij(struct comm_info* c_info, int size, POSITIONING pos, 
                  BTYPES type, int i_sample, int j_sample, 
                  int time_inner, double* time)
======================================================================
/*

                      
                      Calls the proper write functions, blocking case
                      
                      (See IMB_read_ij for documentation of calling sequence)
                      


*/




======================================================================
void IMB_iwrite_ij(struct comm_info* c_info, int size, POSITIONING pos, 
                   BTYPES type, int i_sample, int j_sample, 
                   int time_inner, int do_ovrlp, double* time)
======================================================================
/*

                      
                      Calls the proper write functions, non blocking case
                      
                      (See IMB_read_ij for documentation of calling sequence)
                      


*/


III.4  Organizational functions 
--------------------------------

**********************************************************************
File: IMB.c
**********************************************************************

Implemented function: 

main;




======================================================================
int main(int argc, char **argv)
======================================================================
/*



Input variables: 

-argc                 (type Number of command line arguments)                      
                      int 
                      

-argv                 (type char **)                      
                      List of command line arguments
                      


Return value          (type int)                      
                      0 always
                      


*/


**********************************************************************
File: IMB_init.c
**********************************************************************

Implemented functions: 

IMB_basic_input;
IMB_get_rank_portion;
IMB_init_communicator;
IMB_set_communicator;
IMB_valid;
IMB_set_default;




======================================================================
int IMB_basic_input(struct comm_info* c_info, struct Bench** P_BList, int *argc, 
                    char ***argv, int* NP_min)
======================================================================
/*



Input variables: 

-argc                 (type int *)                      
                      Number of command line arguments
                      

-argv                 (type char ***)                      
                      List of command line arguments
                      


Output variables: 

-NP_min               (type int*)                      
                      Minimum number of processes to run (-npmin command line argument)
                      

-P_BList              (type struct Bench**)                      
                      (For explanation of struct Bench type:
                      describes all aspects of modes of a benchmark;
                      see II. for more information)
                      
                      Address of list of benchmarks to run;
                      list is set up.
                      

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      


*/




======================================================================
void IMB_get_rank_portion(int rank, int NP, int size, 
                          int unit_size, int* pos1, int* pos2)
======================================================================
/*

                      
                      Splits <size> into even contiguous pieces among processes
                      


Input variables: 

-rank                 (type int)                      
                      Process' rank
                      

-NP                   (type int)                      
                      Number of processes
                      

-size                 (type int)                      
                      Portion to split
                      

-unit_size            (type int)                      
                      Base unit for splitting
                      


Output variables: 

-pos1                 (type int*)
-pos2                 (type int*)                      
                      Process' portion is from unit pos1 to pos2
                      


*/




======================================================================
int IMB_init_communicator(struct comm_info* c_info, int NP)
======================================================================
/*



Input variables: 

-NP                   (type int)                      
                      Number of all started processes
                      


In/out variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      
                      Communicator of active processes gets initialized;
                      grouping of processes (in the 'multi' case) in communicators
                      


Return value          (type int)                      
                      Non currently used error exit (value is always 0)
                      


*/




======================================================================
void IMB_set_communicator(struct comm_info *c_info )
======================================================================
/*

                      
                      Performs the actual communicator splitting
                      


In/out variables: 

-c_info               (type struct comm_info *)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      
                      Application communicator gets initialized
                      


*/




======================================================================
int IMB_valid(struct comm_info * c_info, struct Bench* Bmark, int NP)
======================================================================
/*

                      
                      Validates an input Benchmark / NP setting
                      


Input variables: 

-c_info               (type struct comm_info *)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      

-Bmark                (type struct Bench*)                      
                      (For explanation of struct Bench type:
                      describes all aspects of modes of a benchmark;
                      see II. for more information)
                      
                      User input benchmark setting
                      

-NP                   (type int)                      
                      Number of active processes
                      


Return value          (type int)                      
                      1/0 for valid / invalid input
                      


*/




======================================================================
void IMB_set_default(struct comm_info* c_info)
======================================================================
/*

                      
                      Default initialization of comm_info
                      


Output variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      


*/


**********************************************************************
File: IMB_declare.c
**********************************************************************

Implemented function: 



**********************************************************************
File: IMB_parse_name_mpi1.c
**********************************************************************

Implemented functions: 

IMB_get_def_cases;
IMB_set_bmark;




======================================================================
void IMB_get_def_cases(char*** defc, char*** Gcmt)
======================================================================
/*

                      
                      Initializes default benchmark names (defc) and accompanying
                      comments (Gcmt)
                      


In/out variables: 

-defc                 (type char***)                      
                      List of benchkark names (strings)
                      

-Gcmt                 (type char***)                      
                      List of general comments (strings)
                      


*/




======================================================================
void IMB_set_bmark(struct Bench* Bmark)
======================================================================
/*



In/out variables: 

-Bmark                (type struct Bench*)                      
                      (For explanation of struct Bench type:
                      describes all aspects of modes of a benchmark;
                      see II. for more information)
                      
                      On input, only the name of the benchmark is set.
                      On output, all necessary run modes are set accordingly
                      


*/


**********************************************************************
File: IMB_parse_name_ext.c
**********************************************************************

Implemented functions: 

IMB_get_def_cases;
IMB_set_bmark;




======================================================================
void IMB_get_def_cases(char*** defc, char*** Gcmt)
======================================================================
/*

                      
                      Initializes default benchmark names (defc) and accompanying
                      comments (Gcmt)
                      


In/out variables: 

-defc                 (type char***)                      
                      List of benchkark names (strings)
                      

-Gcmt                 (type char***)                      
                      List of general comments (strings)
                      


*/




======================================================================
void IMB_set_bmark(struct Bench* Bmark)
======================================================================
/*



In/out variables: 

-Bmark                (type struct Bench*)                      
                      (For explanation of struct Bench type:
                      describes all aspects of modes of a benchmark;
                      see II. for more information)
                      
                      On input, only the name of the benchmark is set.
                      On output, all necessary run modes are set accordingly
                      


*/


**********************************************************************
File: IMB_parse_name_io.c
**********************************************************************

Implemented functions: 

IMB_get_def_cases;
IMB_set_bmark;




======================================================================
void IMB_get_def_cases(char*** defc, char*** Gcmt)
======================================================================
/*

                      
                      Initializes default benchmark names (defc) and accompanying
                      comments (Gcmt)
                      


In/out variables: 

-defc                 (type char***)                      
                      List of benchkark names (strings)
                      

-Gcmt                 (type char***)                      
                      List of general comments (strings)
                      


*/




======================================================================
void IMB_set_bmark(struct Bench* Bmark)
======================================================================
/*



In/out variables: 

-Bmark                (type struct Bench*)                      
                      (For explanation of struct Bench type:
                      describes all aspects of modes of a benchmark;
                      see II. for more information)
                      
                      On input, only the name of the benchmark is set.
                      On output, all necessary run modes are set accordingly
                      


*/


**********************************************************************
File: IMB_init_file.c
**********************************************************************

Implemented functions: 

IMB_init_file_content;
IMB_init_file;
IMB_free_file;
IMB_del_file;
IMB_open_file;




======================================================================
void IMB_init_file_content(void* BUF, int pos1, int pos2)
======================================================================
/*

                      
                      Initializes contents of a file for READ benchmarks
                      


Input variables: 

-pos1                 (type int)
-pos2                 (type int)                      
                      pos1, pos2: target positions (start/end) in file
                      


In/out variables: 

-BUF                  (type void*)                      
                      Content of buffer to be written to file between these positions
                      


*/




======================================================================
int IMB_init_file(struct comm_info* c_info, struct Bench* Bmark, int NP)
======================================================================
/*



Input variables: 

-Bmark                (type struct Bench*)                      
                      (For explanation of struct Bench type:
                      describes all aspects of modes of a benchmark;
                      see II. for more information)
                      
                      Given file i/o benchmark
                      

-NP                   (type int)                      
                      Number of active processes
                      


In/out variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      
                      MPI_File component is set accordingly
                      


Return value          (type int)                      
                      Error code (identical with MPI error code if occurs)
                      


*/




======================================================================
void IMB_free_file(struct comm_info * c_info)
======================================================================
/*



In/out variables: 

-c_info               (type struct comm_info *)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      
                      File related components are free-d and reset to 
                      NULL initialization
                      


*/




======================================================================
void IMB_del_file(struct comm_info* c_info)
======================================================================
/*



In/out variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      
                      File associated to MPI_File component is erased from disk
                      


*/




======================================================================
int IMB_open_file(struct comm_info* c_info)
======================================================================
/*



In/out variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      
                      File associated to MPI_File component is opened, view is set
                      


Return value          (type int)                      
                      Error code (identical with MPI error code if occurs)
                      


*/


**********************************************************************
File: IMB_user_set_info.c
**********************************************************************

Implemented function: 

IMB_user_set_info;




======================================================================
void IMB_user_set_info(MPI_Info* opt_info)
======================================================================
/*



In/out variables: 

-opt_info             (type MPI_Info*)                      
                      Is set. Default is MPI_INFO_NULL, everything beyond
                      this is user decision and system dependent.
                      


*/


**********************************************************************
File: IMB_benchlist.c
**********************************************************************

Implemented functions: 

IMB_list_names;
IMB_get_def_index;
IMB_construct_blist;
IMB_destruct_blist;
IMB_print_blist;




======================================================================
void IMB_list_names(char* Bname, int** List)
======================================================================
/*



Input variables: 

-Bname                (type char*)                      
                      Input benchmark name (or "all" for all available benchmarks)
                      


In/out variables: 

-List                 (type int**)                      
                      Auxiliary list of internal numbering for input benchmark(s)
                      


*/




======================================================================
void IMB_get_def_index(int* index, char* name)
======================================================================
/*



Input variables: 

-name                 (type char*)                      
                      Input benchmark name
                      


In/out variables: 

-index                (type int*)                      
                      Internal number of benchmark
                      


*/




======================================================================
void IMB_construct_blist(struct Bench** P_BList, int n_args, char* name)
======================================================================
/*

                      
                      Sets up the list of requested benchmarks 
                      (represented as list of struct Bench structures).
                      In one call, 1 benchmark is included.
                      


Input variables: 

-n_args               (type int)                      
                      Overall number of benchmarks to be run (0 means "all")
                      

-name                 (type char*)                      
                      Name of benchmark to be included in list
                      


Output variables: 

-P_BList              (type struct Bench**)                      
                      (For explanation of struct Bench type:
                      describes all aspects of modes of a benchmark;
                      see II. for more information)
                      
                      Updated benchmark list
                      


*/




======================================================================
void IMB_destruct_blist(struct Bench ** P_BList)
======================================================================
/*

                      
                      Completely destructs benchmark list
                      


In/out variables: 

-P_BList              (type struct Bench **)                      
                      (For explanation of struct Bench type:
                      describes all aspects of modes of a benchmark;
                      see II. for more information)
                      
                      All substructures plus list itself are free-d
                      and NULL initialized
                      


*/




======================================================================
void IMB_print_blist(struct comm_info * c_info, struct Bench *BList)
======================================================================
/*

                      
                      Displays requested benchmark scenario on stdout
                      


Input variables: 

-c_info               (type struct comm_info *)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      

-BList                (type struct Bench *)                      
                      (For explanation of struct Bench type:
                      describes all aspects of modes of a benchmark;
                      see II. for more information)
                      


*/


**********************************************************************
File: IMB_init_transfer.c
**********************************************************************

Implemented functions: 

IMB_init_transfer;
IMB_close_transfer;




======================================================================
void IMB_init_transfer(struct comm_info* c_info, struct Bench* Bmark, int size)
======================================================================
/*

                      
                      For IO  case: file splitting/view is set, file is opened
                      For EXT case: window is created and synchronized (MPI_Win_fence)
                      


Input variables: 

-Bmark                (type struct Bench*)                      
                      (For explanation of struct Bench type:
                      describes all aspects of modes of a benchmark;
                      see II. for more information)
                      
                      Given benchmark
                      

-size                 (type int)                      
                      (Only IO case): used to determine file view
                      


In/out variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      
                      Corresponding components (File or Window related) are set
                      


*/




======================================================================
void IMB_close_transfer (struct comm_info* c_info, struct Bench* Bmark, int size)
======================================================================
/*

                      
                      Closes / frees file / window components
                      


Input variables: 

-Bmark                (type struct Bench*)                      
                      (For explanation of struct Bench type:
                      describes all aspects of modes of a benchmark;
                      see II. for more information)
                      
                      Given benchmark
                      

-size                 (type int)                      
                      (Only IO case): used to determine file view
                      


In/out variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      
                      Corresponding components (File or Window related) are freed
                      


*/


**********************************************************************
File: IMB_warm_up.c
**********************************************************************

Implemented function: 

IMB_warm_up;




======================================================================
void IMB_warm_up (struct comm_info* c_info, struct Bench* Bmark, int iter)
======================================================================
/*

                      
                      'Warm up' run of the particular benchmark, so the
                      system can eventually set up internal structures before
                      the actual benchmark
                      


Input variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      

-Bmark                (type struct Bench*)                      
                      (For explanation of struct Bench type:
                      describes all aspects of modes of a benchmark;
                      see II. for more information)
                      
                      The actual benchmark
                      

-iter                 (type int)                      
                      Number of the outer iteration of the benchmark. Only
                      for iter==0, the WamrUp is carried out
                      


*/


**********************************************************************
File: IMB_cpu_exploit.c
**********************************************************************

Implemented function: 

IMB_cpu_exploit;




======================================================================
void IMB_cpu_exploit(float target_secs, int initialize)
======================================================================
/*

                      
                      Runs a CPU intensive code (matrix multiply) for a
                      user defined amount of CPU time
                      


Input variables: 

-target_secs          (type float)                      
                      That many seconds (about) the matrix multiply is run
                      

-initialize           (type int)                      
                      1/0 for first / subsequent calls. If 1, the program
                      determines how to run in order to achieve target CPU time.
                      


*/


**********************************************************************
File: IMB_g_info.c
**********************************************************************

Implemented functions: 

IMB_general_info;
IMB_make_sys_info;
IMB_end_msg;




======================================================================
void IMB_general_info()
======================================================================
/*

                      
                      Prints to stdout some basic information 
                      (Version, time, system (see 'IMB_make_sys_info'))
                      


*/




======================================================================
void IMB_make_sys_info()
======================================================================
/*

                      
                      Prints to stdout some basic information about the system
                      (outcome of the 'uname' command)
                      


*/




======================================================================
void IMB_end_msg(struct comm_info* c_info )
======================================================================
/*

                      
                      Prints to stdout an eventual end message (currently empty)
                      


Input variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      


*/


**********************************************************************
File: IMB_output.c
**********************************************************************

Implemented functions: 

IMB_output;
IMB_display_times;
IMB_show_selections;
IMB_show_procids;
IMB_print_array;
IMB_print_int_row;
IMB_print_info;
IMB_print_headlines;
IMB_edit_format;
IMB_make_line;




======================================================================
void IMB_output(struct comm_info* c_info, struct Bench* Bmark, MODES BMODE, 
                int header, int size, int n_sample, 
                double *time)
======================================================================
/*



Input variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      

-Bmark                (type struct Bench*)                      
                      (For explanation of struct Bench type:
                      describes all aspects of modes of a benchmark;
                      see II. for more information)
                      
                      The actual benchmark
                      

-BMODE                (type MODES)                      
                      The actual benchmark mode (if relevant; only MPI-2 case, see II.)
                      

-header               (type int)                      
                      1/0 for do/don't print table headers
                      

-size                 (type int)                      
                      Benchmark message size
                      

-n_sample             (type int)                      
                      Benchmark repetition number
                      

-time                 (type double *)                      
                      Benchmark timing outcome
                      3 numbers (min/max/average)
                      


*/




======================================================================
void IMB_display_times(struct Bench* Bmark, double* tlist, struct comm_info* c_info, 
                       int group, int n_sample, int size, 
                       int edit_type)
======================================================================
/*



Input variables: 

-Bmark                (type struct Bench*)                      
                      (For explanation of struct Bench type:
                      describes all aspects of modes of a benchmark;
                      see II. for more information)
                      
                      The actual benchmark
                      

-tlist                (type double*)                      
                      Benchmark timing outcome
                      3 numbers (min/max/average)
                      

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      

-group                (type int)                      
                      Index of group to be displayed (multi-case only)
                      

-n_sample             (type int)                      
                      Benchmark repetition number
                      

-size                 (type int)                      
                      Benchmark message size
                      

-edit_type            (type int)                      
                      Code for table formatting details
                      


*/




======================================================================
void IMB_show_selections(struct comm_info* c_info, struct Bench* BList)
======================================================================
/*

                      
                      Displays on stdout an overview of the user selections
                      


Input variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      

-BList                (type struct Bench*)                      
                      (For explanation of struct Bench type:
                      describes all aspects of modes of a benchmark;
                      see II. for more information)
                      
                      The requested list of benchmarks
                      


*/




======================================================================
void IMB_show_procids(struct comm_info* c_info)
======================================================================
/*

                      
                      Prints to stdout the process ids (of group eventually)
                      


Input variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      


*/




======================================================================
void IMB_print_array(int* Array, int N, int disp_N, 
                     int M, char* txt, FILE* unit)
======================================================================
/*

                      
                      Formattedly prints to stdout a M by N int array 
                      


Input variables: 

-Array                (type int*)                      
                      Array to be printed
                      

-N                    (type int)                      
                      Number of rows to be printed
                      

-disp_N               (type int)                      
                      Displacement in Array where frist row begins
                      

-M                    (type int)                      
                      Number of columns
                      

-txt                  (type char*)                      
                      Accompanying text
                      

-unit                 (type FILE*)                      
                      Output unit
                      


*/




======================================================================
void IMB_print_int_row(FILE* unit, int* Array, int M)
======================================================================
/*

                      
                      Formattedly prints to stdout a row of int numbers
                      


Input variables: 

-unit                 (type FILE*)                      
                      Output unit
                      

-Array                (type int*)                      
                      Data to be printed
                      

-M                    (type int)                      
                      Number of data
                      


*/




======================================================================
void IMB_print_info()
======================================================================
/*

                      
                      Prints MPI_Info selections (MPI-2 only)
                      


*/




======================================================================
void IMB_print_headlines(char* h_line)
======================================================================
/*

                      
                      Prints header legend of table
                      


Input variables: 

-h_line               (type char*)                      
                      Header legend, single items separated by '&'
                      


*/




======================================================================
void IMB_edit_format(int n_ints , int n_floats)
======================================================================
/*

                      
                      Edits format string for output
                      


In/out variables: 

-n_ints               (type int)                      
                      # of int items to be printed
                      

-n_floats             (type int)                      
                      # of float items to be printed
                      


*/




======================================================================
void IMB_make_line(int li_len)
======================================================================
/*

                      
                      Prints an underline
                      


Input variables: 

-li_len               (type int)                      
                      Length of underline
                      


*/


**********************************************************************
File: IMB_mem_manager.c
**********************************************************************

Implemented functions: 

IMB_v_alloc;
IMB_i_alloc;
IMB_alloc_buf;
IMB_alloc_aux;
IMB_free_aux;
IMB_v_free;
IMB_ass_buf;
IMB_set_buf;
IMB_init_pointers;
IMB_init_buffers;
IMB_free_all;
IMB_del_s_buf;
IMB_del_r_buf;




======================================================================
void* IMB_v_alloc(int Len, char* where)
======================================================================
/*

                      
                      Allocates void* memory
                      


Input variables: 

-Len                  (type int)                      
                      #bytes to allocate
                      

-where                (type char*)                      
                      Comment (marker for calling place)
                      


Return value          (type void*)                      
                      Allocated pointer
                      


*/




======================================================================
void IMB_i_alloc(int** B, int Len, char* where )
======================================================================
/*

                      
                      Allocates int memory
                      


Input variables: 

-Len                  (type int)                      
                      #int's to allocate
                      

-where                (type char*)                      
                      Comment (marker for calling place)
                      


In/out variables: 

-B                    (type int**)                      
                      *B contains allocated memory
                      


*/




======================================================================
void IMB_alloc_buf(struct comm_info* c_info, char* where, int s_len, 
                   int r_len)
======================================================================
/*

                      
                      Allocates send/recv buffers for message passing
                      


Input variables: 

-where                (type char*)                      
                      Comment (marker for calling place)
                      

-s_len                (type int)                      
                      Send buffer length (bytes)
                      

-r_len                (type int)                      
                      Recv buffer length (bytes)
                      


In/out variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      
                      Send/Recv buffer components get allocated
                      


*/




======================================================================
void IMB_alloc_aux(int L, char* where)
======================================================================
/*

                      
                      Allocates global auxiliary memory AUX
                      


Input variables: 

-L                    (type int)                      
                      #Bytes to allocate
                      

-where                (type char*)                      
                      Comment (marker for calling place)
                      


*/




======================================================================
void IMB_free_aux()
======================================================================
/*

                      
                      Free-s global auxiliary memory AUX
                      


*/




======================================================================
void IMB_v_free(void **B)
======================================================================
/*

                      
                      Free-s memory
                      


In/out variables: 

-B                    (type void**)                      
                      (*B) will be free-d
                      


*/




======================================================================
void IMB_ass_buf(void* buf, int rank, int pos1, 
                 int pos2, int value)
======================================================================
/*

                      
                      Assigns values to a buffer
                      


Input variables: 

-rank                 (type int)                      
                      Rank of calling process
                      

-pos1                 (type int)
-pos2                 (type int)                      
                      Assignment between byte positions pos1, pos2 
                      

-value                (type int)                      
                      1/0 for non-zero (defined in IMB_settings.h)/ zero value
                      


In/out variables: 

-buf                  (type void*)                      
                      Values assigned within given positions
                      


*/




======================================================================
void IMB_set_buf(struct comm_info* c_info, int selected_rank, int s_pos1, 
                 int s_pos2, int r_pos1, int r_pos2)
======================================================================
/*

                      
                      Sets Send/Recv buffers for a selected rank
                      (by call to => IMB_ass_buf)
                      


Input variables: 

-selected_rank        (type int)                      
                      Relevant process rank
                      (Can be different from local rank: for checking purposes)
                      

-s_pos1               (type int)
-s_pos2               (type int)                      
                      s_pos1 .. s_pos2 positions for send buffer
                      

-r_pos1               (type int)
-r_pos2               (type int)                      
                      r_pos1 .. r_pos2 positions for recv buffer
                      


In/out variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      
                      Corresponding buffer components are assigned values
                      


*/




======================================================================
void IMB_init_pointers(struct comm_info *c_info )
======================================================================
/*

                      
                      Initializes pointer components of comm_info
                      


In/out variables: 

-c_info               (type struct comm_info *)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      
                      Corresponding pointer components are initialized
                      


*/




======================================================================
void IMB_init_buffers(struct comm_info* c_info, struct Bench* Bmark, int size)
======================================================================
/*

                      
                      Initializes communications buffers (call set_buf)
                      


Input variables: 

-Bmark                (type struct Bench*)                      
                      (For explanation of struct Bench type:
                      describes all aspects of modes of a benchmark;
                      see II. for more information)
                      
                      Current benchmark
                      

-size                 (type int)                      
                      Message size
                      


In/out variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      
                      Communications buffers are allocated and assigned values
                      


*/




======================================================================
void IMB_free_all(struct comm_info* c_info, struct Bench** P_BList)
======================================================================
/*

                      
                      Free-s all allocated memory in c_info and P_Blist
                      


In/out variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      

-P_BList              (type struct Bench**)                      
                      (For explanation of struct Bench type:
                      describes all aspects of modes of a benchmark;
                      see II. for more information)
                      


*/




======================================================================
void IMB_del_s_buf(struct comm_info* c_info )
======================================================================
/*

                      
                      Deletes send buffer component of c_info
                      


In/out variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      


*/




======================================================================
void IMB_del_r_buf(struct comm_info* c_info )
======================================================================
/*

                      
                      Deletes recv buffer component of c_info
                      


In/out variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      


*/


**********************************************************************
File: IMB_strgs.c
**********************************************************************

Implemented functions: 

IMB_str;
IMB_lwr;
IMB_str_atoi;
IMB_str_erase;




======================================================================
char* IMB_str(char* Bname)
======================================================================
/*

                      
                      Copies string Bname to new memory which is returned
                      


Input variables: 

-Bname                (type char*)                      
                      String to be copied
                      


Return value          (type char*)                      
                      Copy of Bname with newly allocated memory
                      


*/




======================================================================
void IMB_lwr(char* Bname)
======================================================================
/*



In/out variables: 

-Bname                (type char*)                      
                      Uper case alphabetic characters are converted to lower case
                      


*/




======================================================================
int IMB_str_atoi(char s[])
======================================================================
/*

                      
                      Evaluates int value of a numeric string
                      


Input variables: 

-s                    (type char [])                      
                      String with only numeric characters
                      


Return value          (type int)                      
                      Numeric value
                      


*/




======================================================================
void IMB_str_erase(char* string, int Nblnc)
======================================================================
/*

                      
                      Fills blancs into a string
                      


Input variables: 

-Nblnc                (type int)                      
                      #blancs to fill
                      


In/out variables: 

-string               (type char*)                      
                      Null terminated string with Nblnc many blancs
                      


*/


**********************************************************************
File: IMB_err_handler.c
**********************************************************************

Implemented functions: 

IMB_err_hand;
IMB_errors_mpi;
IMB_errors_win;
IMB_errors_io;
IMB_init_errhand;
IMB_set_errhand;
IMB_del_errhand;




======================================================================
void IMB_err_hand(int ERR_IS_MPI, int ERR_CODE )
======================================================================
/*

                      
                      Handles input error code. 
                      Retrieves error string (MPI_Error_string) if is an MPI error code
                      Calls MPI_Abort
                      


Input variables: 

-ERR_IS_MPI           (type int)                      
                      Logical flag: error code belongs to MPI or not
                      

-ERR_CODE             (type int)                      
                      Input error code. If an MPI error code, the string is retrieved.
                      Anyway MPI_Abort is called
                      


*/




======================================================================
void IMB_errors_mpi(MPI_Comm * comm, int* ierr, ...)
======================================================================
/*

                      
                      Error handler callback for MPI-1 errors
                      


Input variables: 

-comm                 (type MPI_Comm *)                      
                      Communicator which is in error
                      

-ierr                 (type int*)                      
                      MPI error code
                      


*/




======================================================================
void IMB_errors_win(MPI_Win * WIN, int* ierr, ...)
======================================================================
/*

                      
                      Error handler callback for onesided communications errors
                      


Input variables: 

-WIN                  (type MPI_Win *)                      
                      MPI Window which is in error
                      

-ierr                 (type int*)                      
                      MPI error code
                      


*/




======================================================================
void IMB_errors_io (MPI_File * fh, int* ierr, ...)
======================================================================
/*

                      
                      Error handler callback for MPI-IO errors
                      


Input variables: 

-fh                   (type MPI_File *)                      
                      MPI File which is in error
                      

-ierr                 (type int*)                      
                      MPI error code
                      


*/




======================================================================
void IMB_init_errhand(struct comm_info* c_info)
======================================================================
/*

                      
                      Creates MPI error handler component of c_info by MPI_<>_create_errhandler
                      


In/out variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      
                      Error handler component is created 
                      (c_info->ERR for MPI-1; c_info->ERRW for EXT; c_info->ERRF for MPIIO)
                      


*/




======================================================================
void IMB_set_errhand(struct comm_info* c_info)
======================================================================
/*

                      
                      Sets MPI error handler component of c_info by MPI_<>_set_errhandler
                      


In/out variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      
                      Error handler component is set
                      


*/




======================================================================
void IMB_del_errhand(struct comm_info* c_info)
======================================================================
/*

                      
                      Deletes MPI error handler component of c_info by MPI_Errhandler_free
                      


In/out variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      
                      MPI error handler component of c_info is deleted
                      


*/


**********************************************************************
File: IMB_chk_diff.c
**********************************************************************

Implemented functions: 

IMB_chk_dadd;
IMB_ddiff;
IMB_show;
IMB_err_msg;
IMB_chk_diff;
IMB_cmp_cat;
IMB_chk_contiguous;
IMB_chk_distr;
IMB_chk_contained;
IMB_compute_crc;




======================================================================
void IMB_chk_dadd(void* AUX, int Locsize, int buf_pos, 
                  int rank0, int rank1)
======================================================================
/*

                      
                      Auxiliary, only for checking mode; 
                      Creates reference accumulated values in a given 
                      buffer section, accumulated over given processor ranks 
                      


Input variables: 

-Locsize              (type int)                      
                      Size of buffer section to check
                      

-buf_pos              (type int)                      
                      Start position of buffer section
                      

-rank0                (type int)
-rank1                (type int)                      
                      Process' values between rank0 and rank1 are accumulated
                      


In/out variables: 

-AUX                  (type void*)                      
                      Contains accumulated values
                      


*/




======================================================================
double IMB_ddiff(assign_type *A, assign_type *B, int len, 
                 int *fault_pos)
======================================================================
/*

                      
                      Compares the values of 2 buffers A, B and returns max. diff
                      


Input variables: 

-A                    (type assign_type *)                      
                      Buffer of values
                      

-B                    (type assign_type *)                      
                      Another buffer of values to be checked against A
                      

-len                  (type int)                      
                      Length (in assign_type items) of A, B
                      


Output variables: 

-fault_pos            (type int *)                      
                      Position of first non tolerable deviation
                      


Return value          (type double)                      
                      Deviation of A and B
                      


*/




======================================================================
void IMB_show(char* text, struct comm_info* c_info, void* buf, 
              int loclen, int totlen, int j_sample, 
              POSITIONING fpos)
======================================================================
/*

                      
                      Shows an excerpt of erroneous buffer if occurs in check mode
                      


Input variables: 

-text                 (type char*)                      
                      Accompanying text to put out
                      

-loclen               (type int)                      
                      Local length of buffer
                      

-totlen               (type int)                      
                      Total length of buffer (for gathered or shared access buffers)
                      

-j_sample             (type int)                      
                      Number of sample the error occurred
                      

-fpos                 (type POSITIONING)                      
                      File positionning (if relevant)
                      

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      

-buf                  (type void*)                      
                      Given check buffer
                      


*/




======================================================================
void IMB_err_msg(struct comm_info* c_info, char* text, int totsize, 
                 int j_sample)
======================================================================
/*

                      
                      Outputs an brief error diagnostics if occurs
                      


Input variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      

-text                 (type char*)                      
                      Accompanying text
                      

-totsize              (type int)                      
                      Size of the erroneous buffer
                      

-j_sample             (type int)                      
                      Number of sample the error occured in
                      


*/




======================================================================
void IMB_chk_diff(char* text, struct comm_info* c_info, void* RECEIVED, 
                  int buf_pos, int Locsize, int Totalsize, 
                  int unit_size, DIRECTION mode, POSITIONING fpos, 
                  int n_sample, int j_sample, int source, 
                  double* diff )
======================================================================
/*

                      
                      Checks a received buffer against expected ref values
                      


Input variables: 

-text                 (type char*)                      
                      Accompanying text
                      

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      

-RECEIVED             (type void*)                      
                      The buffer to be checked
                      

-buf_pos              (type int)                      
                      Beginning position (in units -> unit_size)
                      

-Locsize              (type int)                      
                      Local buffer size
                      

-Totalsize            (type int)                      
                      Total buffer size (in case of gathered buffers)
                      

-unit_size            (type int)                      
                      Base unit for positioning
                      

-mode                 (type DIRECTION)                      
                      Direction of the action that took place
                      

-fpos                 (type POSITIONING)                      
                      File positioning of the action that took place (if relevant)
                      

-n_sample             (type int)                      
                      # overall samples
                      

-j_sample             (type int)                      
                      current sample
                      

-source               (type int)                      
                      Sending process (if relevant)
                      


Output variables: 

-diff                 (type double*)                      
                      The error against expected values
                      


*/




======================================================================
void IMB_cmp_cat(struct comm_info *c_info, void* RECEIVED, int size, 
                 int bufpos, int unit_size, int perm, 
                 int* lengths, int*ranks, int* Npos, 
                 int *faultpos, double* diff)
======================================================================
/*

                      
                      Checks a received buffer which is a concatenation of 
                      several processes' buffers
                      


Input variables: 

-c_info               (type struct comm_info *)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      

-RECEIVED             (type void*)                      
                      The buffer to be checked
                      

-size                 (type int)                      
                      Size of the buffer
                      

-bufpos               (type int)                      
                      First position to check (in units -> unit_size)
                      

-unit_size            (type int)                      
                      Base unit of positioning
                      

-perm                 (type int)                      
                      Logical flag: 1 iff the different rank's portions
                      are potentially in non natural order (relevant for
                      shared file accesses)
                      


Output variables: 

-lengths              (type int*)                      
                      An array of lengths (of a number of erroneous portions)
                      

-ranks                (type int*)                      
                      An array of ranks (the erroneous portions belong to)
                      

-Npos                 (type int*)                      
                      Numer of erroneous portions found (=size of 'lengths' and 'ranks' arrays)
                      

-faultpos             (type int *)                      
                      Position of first found fault
                      

-diff                 (type double*)                      
                      Diff value
                      


*/




======================================================================
void IMB_chk_contiguous(struct comm_info *c_info, int* displs, int* sizes, 
                        double*diff)
======================================================================
/*

                      
                      Checks whether arrays of displacements/sizes form a
                      contiguous buffer
                      


Input variables: 

-c_info               (type struct comm_info *)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      

-displs               (type int*)                      
                      Array of displacements (one for each process)
                      

-sizes                (type int*)                      
                      Array of sizes (one for each process)
                      


Output variables: 

-diff                 (type double*)                      
                      0 if contiguous, 1 else
                      


*/




======================================================================
void IMB_chk_distr(struct comm_info *c_info, int size, int n_sample, 
                   int* lengths, int* ranks, int Npos, 
                   double *diff)
======================================================================
/*

                      
                      (Only for MPI-IO shared file pointer accesses)
                      Checks whether a found set of section lengths/ranks in
                      a file meets expectations
                      


Input variables: 

-c_info               (type struct comm_info *)                      
                      Collection of all base data for MPI;
                      see II. for more information
                      

-size                 (type int)                      
                      Size of buffer
                      

-n_sample             (type int)                      
                      Number of samples expected in file
                      

-lengths              (type int*)                      
                      Array of section lengths found
                      

-ranks                (type int*)                      
                      Array of ranks belonging to sections
                      

-Npos                 (type int)                      
                      Number of sections
                      


Output variables: 

-diff                 (type double *)                      
                      0 if set is consistent, 1 else
                      


*/




======================================================================
void IMB_chk_contained(void* part, int p_size, void* whole, 
                       int w_size, int* pos, int* fpos, 
                       double* D, char*msg)
======================================================================
/*

                      
                      Checks whether a buffer part is contained in a larger buffer
                      (exploits uniqueness of buffer values, so check is trivial)
                      


Input variables: 

-part                 (type void*)                      
                      Partial buffer
                      

-p_size               (type int)                      
                      Size of partial buffer
                      

-whole                (type void*)                      
                      Whole buffer
                      

-w_size               (type int)                      
                      Size of whole buffer
                      

-msg                  (type char*)                      
                      Accompanying message
                      


Output variables: 

-pos                  (type int*)                      
                      Position where partial buffer begins in whole buffer
                      if search was successful
                      

-fpos                 (type int*)                      
                      Position where first fault occurred when start position was
                      found, but later an error occurred
                      

-D                    (type double*)                      
                      0 if check positive, 1 else
                      


*/




======================================================================
long IMB_compute_crc (register char* buf, register int size)
======================================================================
/*



In/out variables: 

-buf                  (type register char*)
-size                 (type register int)

Return value          (type long)

*/
























