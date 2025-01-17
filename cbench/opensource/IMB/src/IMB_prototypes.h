/*****************************************************************************
 *                                                                           *
 * Copyright (c) 2003-2007 Intel Corporation.                                *
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

***************************************************************************/

#include "IMB_benchmark.h"

/* MPI-1 message passing benchmarks */


void IMB_allgather(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS,
                   MODES RUN_MODE, double* time);

void IMB_allgatherv(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS,
                    MODES RUN_MODE, double* time);

void IMB_gather(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS,
                MODES RUN_MODE, double* time);

void IMB_gatherv(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS,
                 MODES RUN_MODE, double* time);

void IMB_scatter(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS,
                 MODES RUN_MODE, double* time);

void IMB_scatterv(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS,
                  MODES RUN_MODE, double* time);

void IMB_allreduce(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS, 
                   MODES RUN_MODE, double* time);

void IMB_alltoall(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS, 
                  MODES RUN_MODE, double* time);

void IMB_alltoallv(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS, 
                   MODES RUN_MODE, double* time);

void IMB_barrier(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS, 
                 MODES RUN_MODE, double* time);

void IMB_bcast(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS, 
               MODES RUN_MODE, double* time);

void IMB_exchange(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS, 
                  MODES RUN_MODE, double* time);

void IMB_pingping(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS,
                  MODES RUN_MODE, double* time);

void IMB_pingpong(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS,
                  MODES RUN_MODE, double* time);

void IMB_reduce(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS, 
                MODES RUN_MODE, double* time);

void IMB_reduce_scatter(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS, 
                        MODES RUN_MODE, double* time);

void IMB_sendrecv(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS, 
                  MODES RUN_MODE, double* time);


/* MPI-2 onesided communications benchmarks */

#ifdef EXT

void IMB_accumulate (struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS, 
                     MODES RUN_MODE, double* time);

void IMB_bidir_get(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS, 
                   MODES RUN_MODE, double* time);

void IMB_bidir_put(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS, 
                   MODES RUN_MODE, double* time);

void IMB_unidir_put (struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS, 
                     MODES RUN_MODE, double* time);

void IMB_unidir_get (struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS, 
                     MODES RUN_MODE, double* time);

void IMB_ones_get(struct comm_info* c_info, int s_num, int dest, 
                  int r_num, int sender, int size, 
                  struct iter_schedule* ITERATIONS, double* time);

void IMB_ones_mget(struct comm_info* c_info, int s_num, int dest, 
                   int r_num, int sender, int size, 
                   struct iter_schedule* ITERATIONS, double* time);

void IMB_ones_put(struct comm_info* c_info, int s_num, int dest, 
                  int r_num, int sender, int size, 
                  struct iter_schedule* ITERATIONS, double* time);

void IMB_ones_mput(struct comm_info* c_info, int s_num, int dest, 
                   int r_num, int sender, int size, 
                   struct iter_schedule* ITERATIONS, double* time);

void IMB_window(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS,
                MODES RUN_MODE, double* time);

void IMB_user_set_info(MPI_Info* opt_info);

#endif

/* MPI-IO benchmarks */

#ifdef MPIIO

void IMB_open_close(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS, 
                    MODES RUN_MODE, double* time);

void IMB_read_shared(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS, 
                     MODES RUN_MODE, double* time);

void IMB_read_indv(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS, 
                   MODES RUN_MODE, double* time);

void IMB_read_expl(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS, 
                   MODES RUN_MODE, double* time);

void IMB_read_ij(struct comm_info* c_info, int size, POSITIONING pos, 
                 BTYPES type, int i_sample, int j_sample, 
                 int time_inner, double* time);

void IMB_iread_ij(struct comm_info* c_info, int size, POSITIONING pos, 
                  BTYPES type, int i_sample, int j_sample, 
                  int time_inner, int do_ovrlp, double* time);

void IMB_write_shared(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS, 
                      MODES RUN_MODE, double* time);

void IMB_write_indv(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS, 
                    MODES RUN_MODE, double* time);

void IMB_write_expl(struct comm_info* c_info, int size, struct iter_schedule* ITERATIONS, 
                    MODES RUN_MODE, double* time);

void IMB_write_ij(struct comm_info* c_info, int size, POSITIONING pos, 
                  BTYPES type, int i_sample, int j_sample, 
                  int time_inner, double* time);

void IMB_iwrite_ij(struct comm_info* c_info, int size, POSITIONING pos, 
                   BTYPES type, int i_sample, int j_sample, 
                   int time_inner, int do_ovrlp, double* time);

void IMB_user_set_info(MPI_Info* opt_info);

#endif

/* Organizational functions */


int main(int argc, char **argv);

/* IMB 3.1 << */
int IMB_basic_input(struct comm_info* c_info, struct Bench** P_BList, 
                    struct iter_schedule* ITERATIONS,
                    int *argc, char ***argv, int* NP_min);
/* >> IMB 3.1  */

void IMB_get_rank_portion(int rank, int NP, int size, 
                          int unit_size, int* pos1, int* pos2);

int IMB_init_communicator(struct comm_info* c_info, int NP);

void IMB_set_communicator(struct comm_info *c_info );

int IMB_valid(struct comm_info * c_info, struct Bench* Bmark, int NP);

void IMB_set_default(struct comm_info* c_info);


#ifdef MPI1

void IMB_get_def_cases(char*** defc, char*** Gcmt);

void IMB_set_bmark(struct Bench* Bmark);


#endif

#ifdef EXT

void IMB_get_def_cases(char*** defc, char*** Gcmt);

void IMB_set_bmark(struct Bench* Bmark);


#endif

#ifdef MPIIO

void IMB_get_def_cases(char*** defc, char*** Gcmt);

void IMB_set_bmark(struct Bench* Bmark);

void IMB_init_file_content(void* BUF, int pos1, int pos2);

/* << IMB 3.1 */
int IMB_init_file(struct comm_info* c_info, struct Bench* Bmark, struct iter_schedule* ITERATIONS, int NP);
/* >> IMB 3.1 */

void IMB_free_file(struct comm_info * c_info);

void IMB_del_file(struct comm_info* c_info);

int IMB_open_file(struct comm_info* c_info);

void IMB_user_set_info(MPI_Info* opt_info);


#endif

void IMB_list_names(char* Bname, int** List);

void IMB_get_def_index(int* index, char* name);

void IMB_construct_blist(struct Bench** P_BList, int n_args, char* name);

void IMB_destruct_blist(struct Bench ** P_BList);

void IMB_print_blist(struct comm_info * c_info, struct Bench *BList);

/* IMB 3.1 << */
void IMB_init_transfer(struct comm_info* c_info, struct Bench* Bmark, int size, int acc_size);
/* >> IMB 3.1  */

void IMB_close_transfer (struct comm_info* c_info, struct Bench* Bmark, int size);

void IMB_warm_up (struct comm_info* c_info, struct Bench* Bmark, struct iter_schedule* ITERATIONS, int iter);

void IMB_cpu_exploit(float target_secs, int initialize);

void IMB_general_info();

void IMB_make_sys_info();

void IMB_end_msg(struct comm_info* c_info );

/* IMB 3.1 << */
void IMB_output(struct comm_info* c_info, struct Bench* Bmark, MODES BMODE, 
                int header, int size, struct iter_schedule* ITERATIONS,
                double *time);
/* >> IMB 3.1  */

/* New in IMB_3.0 */
void IMB_help();

void IMB_display_times(struct Bench* Bmark, double* tlist, struct comm_info* c_info, 
                       int group, int n_sample, int size, 
                       int edit_type);

/* IMB 3.1 << */
void IMB_show_selections(struct comm_info* c_info, struct Bench* BList, int *argc, char ***argv);
/* >> IMB 3.1  */

void IMB_show_procids(struct comm_info* c_info);

void IMB_print_array(int* Array, int N, int disp_N, 
                     int M, char* txt, FILE* unit);

void IMB_print_int_row(FILE* unit, int* Array, int M);

void IMB_print_info();

void IMB_print_headlines(char* h_line);

void IMB_edit_format(int n_ints , int n_floats);

void IMB_make_line(int li_len);

void* IMB_v_alloc(int Len, char* where);

void IMB_i_alloc(int** B, int Len, char* where );

void IMB_alloc_buf(struct comm_info* c_info, char* where, int s_len, 
                   int r_len);

void IMB_alloc_aux(int L, char* where);

void IMB_free_aux();

void IMB_v_free(void **B);

void IMB_ass_buf(void* buf, int rank, int pos1, 
                 int pos2, int value);

void IMB_set_buf(struct comm_info* c_info, int selected_rank, int s_pos1, 
                 int s_pos2, int r_pos1, int r_pos2);

void IMB_init_pointers(struct comm_info *c_info );

/* IMB 3.1 << */
void IMB_init_buffers_iter(struct comm_info* c_info, struct iter_schedule* ITERATIONS, 
                           struct Bench* Bmark, MODES BMODE, int iter, int size);

void IMB_free_all(struct comm_info* c_info, struct Bench** P_BList, struct iter_schedule* ITERATIONS);
/* >> IMB 3.1  */

void IMB_del_s_buf(struct comm_info* c_info );

void IMB_del_r_buf(struct comm_info* c_info );

char* IMB_str(char* Bname);

void IMB_lwr(char* Bname);

int IMB_str_atoi(char s[]);

void IMB_str_erase(char* string, int Nblnc);

void IMB_err_hand(int ERR_IS_MPI, int ERR_CODE );

void IMB_errors_mpi(MPI_Comm * comm, int* ierr, ...);

#ifdef EXT
void IMB_errors_win(MPI_Win * WIN, int* ierr, ...);
#endif

#ifdef MPIIO
void IMB_errors_io (MPI_File * fh, int* ierr, ...);
#endif

void IMB_init_errhand(struct comm_info* c_info);

void IMB_set_errhand(struct comm_info* c_info);

void IMB_del_errhand(struct comm_info* c_info);

void IMB_chk_dadd(void* AUX, int Locsize, int buf_pos, 
                  int rank0, int rank1);

double IMB_ddiff(assign_type *A, assign_type *B, int len, 
                 int *fault_pos);

void IMB_show(char* text, struct comm_info* c_info, void* buf, 
              int loclen, int totlen, int j_sample, 
              POSITIONING fpos);

void IMB_err_msg(struct comm_info* c_info, char* text, int totsize, 
                 int j_sample);

void IMB_chk_diff(char* text, struct comm_info* c_info, void* RECEIVED, 
                  int buf_pos, int Locsize, int Totalsize, 
                  int unit_size, DIRECTION mode, POSITIONING fpos, 
                  int n_sample, int j_sample, int source, 
                  double* diff );

void IMB_cmp_cat(struct comm_info *c_info, void* RECEIVED, int size, 
                 int bufpos, int unit_size, int perm, 
                 int* lengths, int*ranks, int* Npos, 
                 int *faultpos, double* diff);

void IMB_chk_contiguous(struct comm_info *c_info, int* rdispl, int* sizes, 
                        double*diff);

void IMB_chk_distr(struct comm_info *c_info, int size, int n_sample, 
                   int* lengths, int* ranks, int Npos, 
                   double *diff);

void IMB_chk_contained(void* part, int p_size, void* whole, 
                       int w_size, int* pos, int* fpos, 
                       double* D, char*msg);

long IMB_compute_crc (register char* buf, register int size);


