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
    
 File: IMB_mem_manager.c 

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

 ***************************************************************************/



#include "IMB_declare.h"
#include "IMB_benchmark.h"

#include "IMB_prototypes.h"




void* IMB_v_alloc(int Len, char* where)
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
{
void* B;
Len=max(asize,Len);
if( (B = (void*)malloc(Len) ) == NULL )
  {
printf ("Memory allocation failed. code position: %s. tried to alloc. %d bytes\n",where,Len);
return NULL;
  }
return B;
}




void IMB_i_alloc(int** B, int Len, char* where )
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
{
Len=max(1,Len);
*B = (int*) IMB_v_alloc(sizeof(int)*Len, where);
}




void IMB_alloc_buf(struct comm_info* c_info, char* where, int s_len, 
                   int r_len)
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
                      see [1] for more information
                      
                      Send/Recv buffer components get allocated
                      


*/
{
/* July 2002 V2.2.1 change: use MPI_Alloc_mem */
#if ( defined EXT || defined MPIIO )
  MPI_Aint slen = (MPI_Aint)(max(1,s_len));
  MPI_Aint rlen = (MPI_Aint)(max(1,r_len));
  int ierr;
#else
  s_len=max(1,s_len);
  r_len=max(1,r_len);
#endif
  if( c_info->s_alloc < s_len )
         {
         if( c_info->s_alloc > 0 ) 
/* July 2002 V2.2.1 change: use MPI_Alloc_mem */
#if ( defined EXT || defined MPIIO )
                { MPI_Free_mem(c_info->s_buffer); }

         ierr=MPI_Alloc_mem(slen, MPI_INFO_NULL, &c_info->s_buffer);
         MPI_ERRHAND(ierr);
#else
                { free(c_info->s_buffer); }

         c_info->s_buffer = IMB_v_alloc(s_len,where);
#endif

         c_info->s_alloc = s_len;
         c_info->s_data = (assign_type*)c_info->s_buffer;
         }
  if( c_info->r_alloc < r_len )
         {
         if( c_info->r_alloc > 0 ) 
/* July 2002 V2.2.1 change: use MPI_Alloc_mem */
#if ( defined EXT || defined MPIIO )
                { MPI_Free_mem(c_info->r_buffer); }

         ierr=MPI_Alloc_mem(rlen, MPI_INFO_NULL, &c_info->r_buffer);
         MPI_ERRHAND(ierr);
#else
                { free(c_info->r_buffer); }

         c_info->r_buffer = IMB_v_alloc(r_len,where);
#endif

         c_info->r_alloc = r_len;
         c_info->r_data = (assign_type*)c_info->r_buffer;
         }

}




void IMB_alloc_aux(int L, char* where)
/*

                      
                      Allocates global auxiliary memory AUX
                      


Input variables: 

-L                    (type int)                      
                      #Bytes to allocate
                      

-where                (type char*)                      
                      Comment (marker for calling place)
                      


*/
{
L=max(asize,L);
if( AUX_LEN < L)
  {
  if( AUX_LEN>0 ) free(AUX);

  AUX = IMB_v_alloc(L, where);
  AUX_LEN=L;
  }
}




void IMB_free_aux()
/*

                      
                      Free-s global auxiliary memory AUX
                      


*/
{
if (AUX_LEN > 0 ) {free(AUX); AUX_LEN=0; }
}


void IMB_v_free(void **B)
/*

                      
                      Free-s memory
                      


In/out variables: 

-B                    (type void**)                      
                      (*B) will be free-d
                      


*/
{
if( *B ) free(*B);
*B=NULL;
}




void IMB_ass_buf(void* buf, int rank, int pos1, 
                 int pos2, int value)
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
{

if( pos2>= pos1 )
{
int a_pos1, a_pos2, i, j;
a_pos1 =  pos1/asize;
if( pos2>=pos1 )
a_pos2 =  pos2/asize;
else
a_pos2 =  a_pos1-1;

if( value )
for ( i=a_pos1,j=0 ; i<=a_pos2; i++,j++ )
((assign_type *)buf)[j] = BUF_VALUE(rank,i);

else
for ( i=a_pos1,j=0 ; i<=a_pos2; i++,j++ )
((assign_type *)buf)[j] = 0.;

if( a_pos1*asize != pos1 )
  {
  void* xx = (void*)(((char*)buf)+pos1-a_pos1*asize);
  memmove(buf,xx,pos2-pos1+1); 
  }
}

}




void IMB_set_buf(struct comm_info* c_info, int selected_rank, int s_pos1, 
                 int s_pos2, int r_pos1, int r_pos2)
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
                      see [1] for more information
                      
                      Corresponding buffer components are assigned values
                      


*/
{
/*
Sets c_info->s_buffer/c_info->r_buffer int byte positions 
s_pos1..s_pos2/r_pos1..r_pos2
Values are taken for "selected_rank"
Checks right allocation.
*/
int s_len, r_len;

s_len = (max(s_pos2-s_pos1,0)/asize+1)*asize;
r_len = (max(r_pos2-r_pos1,0)/asize+1)*asize;

IMB_alloc_buf(c_info, "set_buf 1",s_len, r_len);


if( s_pos2 >= s_pos1 ) 
  IMB_ass_buf( c_info->s_buffer, selected_rank, s_pos1, s_pos2, 1);
if( r_pos2 >= r_pos1 ) 
  IMB_ass_buf( c_info->r_buffer, selected_rank, r_pos1, r_pos2, 0);

}



void IMB_init_pointers(struct comm_info *c_info )
/*

                      
                      Initializes pointer components of comm_info
                      


In/out variables: 

-c_info               (type struct comm_info *)                      
                      Collection of all base data for MPI;
                      see [1] for more information
                      
                      Corresponding pointer components are initialized
                      


*/
{
/********************************************************************


---------------------------------------------------------------------
             VARIABLE |       TYPE        |   MEANING
---------------------------------------------------------------------
In/Out     : c_info   | struct comm_info* | see comm_info.h 
                      |                   | Pointers initialized
----------------------------------------------------------------------*/

MPI_Comm_size(MPI_COMM_WORLD,&c_info->w_num_procs);
MPI_Comm_rank(MPI_COMM_WORLD,&c_info->w_rank     );

c_info->s_data_type   = MPI_BYTE;  /* DATA TYPE of SEND    BUFFER    */ 
c_info->r_data_type   = MPI_BYTE;  /* DATA TYPE of RECEIVE BUFFER    */

c_info->op_type       = MPI_SUM;   /* OPERATION TYPE IN Allred       */
c_info->red_data_type = MPI_FLOAT; /* NOTE: NO 'CAST' CHECK IN. IBUF */

c_info -> s_buffer = c_info -> r_buffer = NULL;
c_info -> s_data   = c_info -> r_data   = NULL;
c_info -> s_alloc  = c_info -> r_alloc = 0;

c_info->communicator= MPI_COMM_NULL;

/* Auxiliary space */
IMB_i_alloc(&c_info->g_ranks,c_info->w_num_procs,"Init_Pointers 1");
IMB_i_alloc(&c_info->g_sizes,c_info->w_num_procs,"Init_Pointers 2");

IMB_i_alloc(&c_info->reccnt,c_info->w_num_procs,"Init_Pointers 3");
IMB_i_alloc(&c_info->displs,c_info->w_num_procs,"Init_Pointers 4");

#ifdef MPIIO
c_info->filename = c_info->datarep = (char*)NULL;
c_info->view = MPI_DATATYPE_NULL;
c_info->info = MPI_INFO_NULL;
c_info->fh = MPI_FILE_NULL;
#endif

all_times = NULL;
#ifdef CHECK
all_defect = NULL;
#endif

IMB_init_errhand(c_info);

}


/**********************************************************************/



void IMB_init_buffers(struct comm_info* c_info, struct Bench* Bmark, int size)
/*

                      
                      Initializes communications buffers (call set_buf)
                      


Input variables: 

-Bmark                (type struct Bench*)                      
                      (For explanation of struct Bench type:
                      describes all aspects of modes of a benchmark;
                      see [1] for more information)
                      
                      Current benchmark
                      

-size                 (type int)                      
                      Message size
                      


In/out variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see [1] for more information
                      
                      Communications buffers are allocated and assigned values
                      


*/
{
  int s_len, r_len, maxlen;
  int init_size;

maxlen = 1<<MAXMSGLOG;
#ifdef MPIIO
  init_size = size;
#endif
#ifdef EXT
  init_size = max(maxlen,OVERALL_VOL);
  if( OVERALL_VOL/MSGSPERSAMPLE > maxlen ) init_size = maxlen*MSGSPERSAMPLE;
#endif
#ifdef MPI1
  init_size = maxlen;
#endif


  if(c_info->rank < 0 ) return;

  if(!strcmp(Bmark->name,"Alltoall") )
    {
      s_len = c_info->num_procs*init_size;
      r_len = c_info->num_procs*init_size;
    }
  else if( !strcmp(Bmark->name,"Allgather") || !strcmp(Bmark->name,"Allgatherv") )
    {
      s_len = init_size;
      r_len = c_info->num_procs*init_size;
    }
  else
      s_len = r_len = init_size;

  IMB_set_buf(c_info, c_info->rank, 0, s_len-1, 0, r_len-1);

}

/********************************************************************/



void IMB_free_all(struct comm_info* c_info, struct Bench** P_BList)
/*

                      
                      Free-s all allocated memory in c_info and P_Blist
                      


In/out variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see [1] for more information
                      

-P_BList              (type struct Bench**)                      
                      (For explanation of struct Bench type:
                      describes all aspects of modes of a benchmark;
                      see [1] for more information)
                      


*/
{
  IMB_del_s_buf(c_info);
  IMB_del_r_buf(c_info);

  IMB_v_free((void**)&c_info->msglen);

  IMB_v_free((void**)&c_info->g_sizes);
  IMB_v_free((void**)&c_info->g_ranks);

  IMB_v_free((void**)&c_info->reccnt); 
  IMB_v_free((void**)&c_info->displs);

  if( c_info->communicator != MPI_COMM_NULL && 
      c_info->communicator != MPI_COMM_SELF &&
      c_info->communicator != MPI_COMM_WORLD )
  {
  IMB_del_errhand(c_info);
  MPI_Comm_free(&c_info->communicator);
  }

  IMB_destruct_blist(P_BList);

#ifdef MPIIO
  IMB_free_file(c_info);
#endif
  if( all_times ) {free (all_times); all_times=(double*)NULL;}

#ifdef CHECK
  if( all_defect ) {free (all_defect);all_defect=(double*)NULL;}

  if( AUX_LEN > 0 ) {free(AUX); AUX_LEN = 0;}
#endif

#ifdef DEBUG
fclose(dbg_file);
#endif
}


void IMB_del_s_buf(struct comm_info* c_info )
/*

                      
                      Deletes send buffer component of c_info
                      


In/out variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see [1] for more information
                      


*/
{
/* July 2002 V2.2.1 change: use MPI_Free_mem */
if ( c_info->s_alloc> 0)
#if (defined EXT || defined MPIIO)
 MPI_Free_mem( c_info->s_buffer );
#else
 free( c_info->s_buffer );
#endif

c_info-> s_alloc = 0;
}




void IMB_del_r_buf(struct comm_info* c_info )
/*

                      
                      Deletes recv buffer component of c_info
                      


In/out variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see [1] for more information
                      


*/
{
/* July 2002 V2.2.1 change: use MPI_Free_mem */
if ( c_info->r_alloc> 0)
#if (defined EXT || defined MPIIO)
 MPI_Free_mem( c_info->r_buffer );
#else
 free( c_info->r_buffer );
#endif

c_info-> r_alloc = 0;
}

