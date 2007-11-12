/*****************************************************************************
 *                                                                           *
 * Copyright (c) 2003-2006 Intel Corporation.                                *
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
    
 File: IMB_output.c 

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

New in IMB_3.0:
 IMB_help;

 ***************************************************************************/

#include <string.h>
#include "IMB_declare.h"
#include "IMB_benchmark.h"

#include "IMB_prototypes.h"


/*****************************************************************/



void IMB_output(struct comm_info* c_info, struct Bench* Bmark, MODES BMODE, 
                int header, int size, int n_sample, 
                double *time)
/*



Input variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see [1] for more information
                      

-Bmark                (type struct Bench*)                      
                      (For explanation of struct Bench type:
                      describes all aspects of modes of a benchmark;
                      see [1] for more information)
                      
                      The actual benchmark
                      

-BMODE                (type MODES)                      
                      The actual benchmark mode (if relevant; only MPI-2 case, see [1])
                      

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
{
  double scaled_time[MAX_TIMINGS];
  
  int DO_OUT;
  int GROUP_OUT;
  int i,i_gr;
  int li_len;
  int edit_type;
  
  ierr = 0;

  DO_OUT    = (c_info->w_rank  == 0 );
  GROUP_OUT = (c_info->group_mode > 0 );

  if (DO_OUT) 
    {
/* Fix IMB_1.0.1: NULL all_times before allocation */
      IMB_v_free((void**)&all_times);
      all_times = 
  (double*)IMB_v_alloc(c_info->w_num_procs * Bmark->Ntimes * sizeof(double), 
                  "Output 1");
#ifdef CHECK
      if(!all_defect)
	{
          all_defect = (double*)IMB_v_alloc(c_info->w_num_procs * sizeof(double), 
                  "Output 1");
          for(i=0; i<c_info->w_num_procs; i++) all_defect[i]=0.;
	}
#endif  	  
    }

/* Scale the timings */
  for(i=0; i<Bmark->Ntimes;  i++)
  scaled_time[i] = time[i] * SCALE * Bmark->scale_time;


/* collect all times  */
  ierr=MPI_Gather(scaled_time,Bmark->Ntimes,MPI_DOUBLE,all_times,Bmark->Ntimes,MPI_DOUBLE,0,MPI_COMM_WORLD);
  MPI_ERRHAND(ierr);

#ifdef CHECK      
/* collect all defects */	      
  ierr=MPI_Gather(&defect,1,MPI_DOUBLE,all_defect,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
  MPI_ERRHAND(ierr);

#endif
  if( DO_OUT )
    {
      BTYPES type= Bmark->RUN_MODES[0].type;
      if ( Bmark->RUN_MODES[0].NONBLOCKING )
           edit_type = 4;
      else if ( type == SingleTransfer && c_info->group_mode != 0 )
           edit_type=0;
      else if ( type == ParallelTransfer || type == SingleTransfer )
           edit_type=1;
      else if (type == Collective )
#ifdef MPIIO
           edit_type=1;
#else
           edit_type=2;
#endif
      else 
           edit_type=3;

      if( header )
	{
        fprintf(unit,"\n");            /* FOR GNUPLOT: CURVE SEPERATOR  */


          if( GROUP_OUT ) {strcpy(aux_string,"&Group") ; li_len=1;}
          else            {strcpy(aux_string,"");  li_len=0;}
	  if ( edit_type == 0 )
	    {
	      li_len+=4;
	      strcat(aux_string,"&#bytes&#repetitions&t[usec]&Mbytes/sec&");
	    }
	  else if ( edit_type == 1 )
	    {
	      li_len+=6;
	      strcat(aux_string,
		     "&#bytes&#repetitions&t_min[usec]&t_max[usec]&t_avg[usec]&Mbytes/sec&");
	    }
	  else if ( edit_type == 2 )
	    {
	      li_len+=5;
	      strcat(aux_string,
		     "&#bytes&#repetitions&t_min[usec]&t_max[usec]&t_avg[usec]&");
	    }
	  else if ( edit_type == 3 )
	    {
	      li_len+=4;
	      strcat(aux_string,
		     "&#repetitions&t_min[usec]&t_max[usec]&t_avg[usec]&");
	    }
          else
            {
	      li_len+=6;
	      strcat(aux_string,
		     "&#bytes&#repetitions&t_ovrl[usec]&t_pure[usec]&t_CPU[usec]& overlap[%]&");
            }
#ifdef CHECK
          if( Bmark->RUN_MODES[0].type != Sync &&
              strcmp(Bmark->name,"Window") )
          {
	  li_len+=1;
	  strcat(aux_string,"&defects&");
          }
#endif
        IMB_make_line(li_len);
        if( c_info->n_groups > 1) 
        fprintf(unit,"# Benchmarking Multi-%s ",Bmark->name);
        else
        fprintf(unit,"# Benchmarking %s ",Bmark->name);
        IMB_show_procids(c_info); 

        IMB_make_line(li_len);

        switch(BMODE->AGGREGATE)
          {
          case 1:
          fprintf(unit,"#\n#    MODE: AGGREGATE \n#\n");
          break;
          case 0:
          fprintf(unit,"#\n#    MODE: NON-AGGREGATE \n#\n");
          break;
          }
	  IMB_print_headlines(aux_string);
	}     



      if( GROUP_OUT )
      {


      for( i_gr=0; i_gr<c_info->n_groups; i_gr++ )
	{
	  if(i_gr == 0) fprintf(unit,"\n");

    	  IMB_display_times(Bmark, all_times, c_info, i_gr, n_sample, size, edit_type);
	} 
      }
      else
    	  IMB_display_times(Bmark, all_times, c_info,  0, n_sample, size, edit_type);
    } 
}
/*****************************************************************/



void IMB_display_times(struct Bench* Bmark, double* tlist, struct comm_info* c_info, 
                       int group, int n_sample, int size, 
                       int edit_type)
/*



Input variables: 

-Bmark                (type struct Bench*)                      
                      (For explanation of struct Bench type:
                      describes all aspects of modes of a benchmark;
                      see [1] for more information)
                      
                      The actual benchmark
                      

-tlist                (type double*)                      
                      Benchmark timing outcome
                      3 numbers (min/max/average)
                      

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see [1] for more information
                      

-group                (type int)                      
                      Index of group to be displayed (multi-case only)
                      

-n_sample             (type int)                      
                      Benchmark repetition number
                      

-size                 (type int)                      
                      Benchmark message size
                      

-edit_type            (type int)                      
                      Code for table formatting details
                      


*/
{
  static double MEGA = 1.0/1048576.0;

  double tmax, tmin, tav, t_pure, throughput, overlap; 
#ifdef CHECK
  double defect;
#endif
  int i, ip, itim, inc;
  
  if( c_info->g_sizes[group]<= 0 ) return;

  inc = Bmark->Ntimes;

  for(itim=0; itim < Bmark->Ntimes; itim++ )
  {


  if( c_info->group_mode > 0)
  {
  
  i =0;
  ip=0;
  while( i<group )
    {
      ip+= c_info->g_sizes[i++];
    }
 
    tmin = tlist[ip*inc+itim];
    tmax = 0.;
    tav  = 0.;
#ifdef CHECK
    defect = 0.;
#endif
    for(i=0; i<c_info->g_sizes[group]; i++)
	{
	  tmax  =  max(tmax,tlist[(ip+i)*inc+itim]);
	  tmin  =  min(tmin,tlist[(ip+i)*inc+itim]);
	  tav  += tlist[(ip+i)*inc+itim];
#ifdef CHECK 
          defect = max ( defect, all_defect[c_info->g_ranks[ip+i]] );
#endif 
	}
    tav /= c_info->g_sizes[group];
    }
    else
    {
    ip = 0;
    for( i=0; i<c_info->n_groups; i++ ) ip += c_info->g_sizes[i];
    tmin = tlist[itim];
    tmax = 0.;
    tav  = 0.;
#ifdef CHECK
    defect = 0.;
#endif

    for(i=0; i<ip; i++)
	{
          int rank=c_info->g_ranks[i];
	  tmax  =  max(tmax,tlist[rank*inc+itim]);
	  tmin  =  min(tmin,tlist[rank*inc+itim]);
	  tav  += tlist[rank*inc+itim];
#ifdef CHECK 
          defect = max ( defect, all_defect[rank] );
#endif 
	}
    tav /= ip;
    }

    ip=0;


    if( Bmark->RUN_MODES[0].NONBLOCKING )
      if( itim == 0 )
        {
        t_pure = tmax;
        }
      else
        {
        overlap = 100.*max(0,min(1,(t_pure+tCPU-tmax)/min(t_pure,tCPU)));
        }

    }  /* for (itim .. ) */

    throughput = 0.;
    if( tmax > 0. ) throughput = (Bmark->scale_bw*SCALE*MEGA)*size/tmax;

    
    if(c_info->group_mode > 0 )
      {
	IMB_edit_format(1,0);
	sprintf(aux_string,format,group);
	ip=strlen(aux_string);
      }

    if( edit_type == 0 )
      { 
	IMB_edit_format(2,2);
	sprintf(aux_string+ip,format,size,n_sample,
		tmax,throughput);
      }
    else if ( edit_type == 1 )
      {
	IMB_edit_format(2,4);
	sprintf(aux_string+ip,format,size,n_sample,tmin,tmax,
		tav,throughput);
      }
    else if ( edit_type == 2 )
      {
	IMB_edit_format(2,3);
	sprintf(aux_string+ip,format,size,n_sample,tmin,tmax,tav);
      }
    else if ( edit_type == 3 )
      {
	IMB_edit_format(1,3);
	sprintf(aux_string+ip,format,n_sample,tmin,tmax,tav);
      }
    else if ( edit_type == 4 )
      {
	IMB_edit_format(2,4);
	sprintf(aux_string+ip,format,size,n_sample,tmax,t_pure,tCPU,overlap);
      }

#ifdef CHECK 
    if ( edit_type != 3 && strcmp(Bmark->name,"Window") )
    {
    IMB_edit_format(0,1);
    ip=strlen(aux_string);
    sprintf(aux_string+ip,format,defect);

    if( defect > TOL    ) Bmark->success=0;
    }
#endif
    fprintf(unit,"%s\n",aux_string);
    fflush(unit);

  
}


/************************************************************************/



void IMB_show_selections(struct comm_info* c_info, struct Bench* BList)
/*

                      
                      Displays on stdout an overview of the user selections
                      


Input variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see [1] for more information
                      

-BList                (type struct Bench*)                      
                      (For explanation of struct Bench type:
                      describes all aspects of modes of a benchmark;
                      see [1] for more information)
                      
                      The requested list of benchmarks
                      


*/
{
  if(c_info->w_rank == 0 )
    {
      IMB_general_info();
      fprintf(unit,"#\n");
#ifndef MPIIO
      fprintf(unit,"# Minimum message length in bytes:   %d\n",0);
      fprintf(unit,"# Maximum message length in bytes:   %d\n",1<<MAXMSGLOG);
      fprintf(unit,"#\n");
      fprintf(unit,"# MPI_Datatype                   :   MPI_BYTE \n");
      fprintf(unit,"# MPI_Datatype for reductions    :   MPI_FLOAT\n");
      fprintf(unit,"# MPI_Op                         :   MPI_SUM  \n");
#else
      fprintf(unit,"# Minimum io portion in bytes:   %d\n",0);
      fprintf(unit,"# Maximum io portion in bytes:   %d\n",1<<MAXMSGLOG);
      fprintf(unit,"#\n");
      IMB_print_info();
#endif
      fprintf(unit,"#\n");

#ifdef IMB_OPTIONAL
      fprintf(unit,"#\n\n");
      fprintf(unit,"# !! Attention: results have been achieved in\n");
      fprintf(unit,"# !! IMB_OPTIONAL mode.\n");
      fprintf(unit,"# !! Results may differ from standard case.\n");
      fprintf(unit,"#\n");
#endif

      fprintf(unit,"#\n");

      IMB_print_blist(c_info, BList);

      if( do_nonblocking )
      {
      fprintf(unit,"\n\n# For nonblocking benchmarks:\n\n");
      fprintf(unit,"# Function CPU_Exploit obtains an undisturbed\n");
      fprintf(unit,"# performance of %7.2f MFlops\n",MFlops);         
      }

    }
}

/****************************************************************************/



void IMB_show_procids(struct comm_info* c_info)
/*

                      
                      Prints to stdout the process ids (of group eventually)
                      


Input variables: 

-c_info               (type struct comm_info*)                      
                      Collection of all base data for MPI;
                      see [1] for more information
                      


*/
{
  int ip, py, i, k, idle;
  
  if( c_info->w_rank == 0 )
    {
      if(c_info->n_groups == 1)
        {
        if( c_info->px>1 && c_info->py>1 )
        {
	fprintf(unit,"\n# #processes = %d; rank order (rowwise): \n",
                c_info->num_procs);
        ip=0;
        for( i=0; i<c_info->px && ip<c_info->NP; i++)
         {  
         py = c_info->w_num_procs/c_info->px;
         if( i<c_info->w_num_procs%c_info->px ) py++;
         py = min(py,c_info->NP-ip);
         IMB_print_array(c_info->g_ranks+ip,1,0,py,"",unit);
         fprintf(unit,"\n");
         ip = ip+py;
         }
        }
        else
	fprintf(unit,"\n# #processes = %d \n",c_info->num_procs);

        idle = c_info->w_num_procs-c_info->num_procs;
        }
      if(c_info->n_groups != 1)
	{
	  fprintf(unit,"\n# ( %d groups of %d processes each running simultaneous ) \n",
		  c_info->n_groups,c_info->num_procs); 

          IMB_print_array(c_info->g_ranks,c_info->n_groups,0,
                                      c_info->g_sizes[0],"Group ",unit);
          idle = c_info->w_num_procs - c_info->n_groups*c_info->g_sizes[0];
	}
      if( idle )
        {
        if( idle == 1 )
	fprintf(unit,"# ( %d additional process waiting in MPI_Barrier)\n",idle);
        else
	fprintf(unit,"# ( %d additional processes waiting in MPI_Barrier)\n",idle);
        }
    }

}


void IMB_print_array(int* Array, int N, int disp_N, 
                     int M, char* txt, FILE* unit)
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
{
#define MAX_SHOW 1024
int i,j;

char* outtxt;
int do_out;

do_out=0;
if( txt )
if( strcmp(txt,"") )
 {
 outtxt=(char*) malloc( (strlen(txt)+6)*sizeof(char));
 do_out=1;
 }

if( N<=1 )
{
if( M>MAX_SHOW )
 {
 fprintf(unit,"#  "); 
 IMB_print_int_row(unit, Array, MAX_SHOW/2);
 fprintf(unit," ... "); 
 IMB_print_int_row(unit, &Array[M-MAX_SHOW/2], MAX_SHOW/2);
 }
else
 {
 if( do_out ) fprintf(unit,"# %s",txt); 
 else         fprintf(unit,"# "); 
 IMB_print_int_row(unit, Array, M);
 }
}
else if ( N<=MAX_SHOW )
{
int zero=0, one=1;
for( i=0; i<N; i++) {
                    if( do_out )
                    sprintf(outtxt,"%s %d: ",txt,disp_N+i);
                    else    outtxt=(char*)NULL;
                    IMB_print_array(&Array[i*M], one, zero, M, outtxt, unit);
                    fprintf(unit,"\n");}
}
else
{
int disp;
disp=0;
IMB_print_array(Array, MAX_SHOW/2, disp, M, txt, unit);
fprintf(unit,"#  . \n"); 
fprintf(unit,"#  . \n"); 
disp=N-MAX_SHOW/2;
IMB_print_array(&Array[(N-MAX_SHOW/2)*M], MAX_SHOW/2, disp, M, txt, unit);
}

}

void IMB_print_int_row(FILE* unit, int* Array, int M)
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
{
#define X_PER_ROW 16
int i,j,i0,irest;

irest = M%X_PER_ROW;
for(j=0; j<(M+X_PER_ROW-1)/X_PER_ROW; j++)
 {
 i0=j*X_PER_ROW;
 for(i=0; i<min(M-i0,X_PER_ROW); i++) fprintf(unit," %4d",Array[i0+i]);
 fprintf(unit,"\n# ");
 }
 

}

#ifdef MPIIO

/****************************************************************************/



void IMB_print_info()
/*

                      
                      Prints MPI_Info selections (MPI-2 only)
                      


*/
{
int nkeys,ikey,vlen,exists;
MPI_Info tmp_info;
char key[MPI_MAX_INFO_KEY], *value;
 
IMB_user_set_info(&tmp_info);

/* July 2002 fix V2.2.1: handle NULL case */
if( tmp_info!=MPI_INFO_NULL ) {
/* end change */

MPI_Info_get_nkeys(tmp_info, &nkeys);

if( nkeys > 0) fprintf(unit,"# Got %d Info-keys:\n\n",nkeys);
 
for( ikey=0; ikey<nkeys; ikey++ )
 {
 MPI_Info_get_nthkey(tmp_info, ikey, key);
 
 MPI_Info_get_valuelen(tmp_info, key, &vlen, &exists);
 
 value = (char*)IMB_v_alloc((vlen+1)* sizeof(char), "Print_Info");
 
 MPI_Info_get(tmp_info, key, vlen, value, &exists);
 printf("# %s = \"%s\"\n",key,value);
 
 free (value);
 }

MPI_Info_free(&tmp_info);

/* July 2002 fix V2.2.1: end if */
}
/* end change */
 
}
#endif



/*****************************************************************/



void IMB_print_headlines(char* h_line)
/*

                      
                      Prints header legend of table
                      


Input variables: 

-h_line               (type char*)                      
                      Header legend, single items separated by '&'
                      


*/
{
  char* help;
  char* token;

  help=h_line;
  while(token = strtok(help, "&") )
    {

      sprintf(format,"%%%ds",ow_format);
      fprintf(unit,format,token);
      help = NULL;
    }
  fprintf(unit,"\n");
}

/*****************************************************************/



void IMB_edit_format(int n_ints , int n_floats)
/*

                      
                      Edits format string for output
                      


In/out variables: 

-n_ints               (type int)                      
                      # of int items to be printed
                      

-n_floats             (type int)                      
                      # of float items to be printed
                      


*/
{
  int ip,i;
  
  ip=0;
  for(i=1 ; i<=n_ints; i++)
    {    
      sprintf(&(format[ip]),"%%%dd",ow_format); 
      ip=strlen(format);
    }
  for(i=1 ; i<=n_floats ; i++)
    {
      sprintf(&(format[ip]),"%%%d.2f",ow_format);
      ip=strlen(format);
    }
}

/***************************************************************************/



void IMB_make_line(int li_len)
/*

                      
                      Prints an underline
                      


Input variables: 

-li_len               (type int)                      
                      Length of underline
                      


*/
{
  int i;
  char* char_line = "-";
  fprintf(unit,"#");
  for( i=1;i<li_len*ow_format; i++ )
    {
      fprintf(unit,"%s",char_line);
    }
  fprintf(unit,"\n");	

}
/* New function for IMB_3.0 */
void IMB_help()
{
fflush(stderr);
fflush(unit);

fprintf(unit,"\nCalling sequence:\n\n");

#ifdef MPI1
fprintf(unit,"\n\
IMB-MPI1    [-h{elp}]\n");
#elif defined(EXT)
fprintf(unit,"\n\
IMB-EXT     [-h{elp}]\n");
#elif defined (MPIIO)
fprintf(unit,"\n\
IMB-IO      [-h{elp}]\n");
#endif
fprintf(unit,"\
            [-npmin  <NPmin>]\n\
            [-multi  <MultiMode>]\n\
            [-msglen <Lengths_file>]\n\
            [-map    <PxQ>]\n\
            [-input  <filename>]\n\
            [benchmark1 [,benchmark2 [,...]]]\n\
\n\
where \n\
\n\
- h ( or help) just provides basic help \n\
  (if active, all other arguments are ignored)\n\
\n\
- NPmin is the minimum number of processes to run on\n\
  (then if IMB is started on NP processes, the process numbers \n\
   NPmin, 2*NPmin, ... ,2^k * NPmin < NP, NP are used)\n\
   >>>\n\
   to run on just NP processes, run IMB on NP and select -npmin NP\n\
   <<<\n\
  Default: NPmin=2\n\
\n\
- P,Q are integer numbers with P*Q <= NP\n\
  Enter PxQ with the 2 numbers separated by letter \"x\" and no blancs\n\
  The basic communicator is set up as P by Q process grid\n\
\n\
  If, e.g., one runs on N nodes of X processors each, and inserts\n\
  P=X, Q=N, then the numbering of processes is \"inter node first\".\n\
  Running PingPong with P=X, Q=2 would measure inter-node performance\n\
  (assuming MPI default would apply 'normal' mapping, i.e. fill nodes\n\
  first priority) \n\
\n\
  Default: Q=1\n\
\n\
- MultiMode is 0 or 1\n\
\n\
  if -multi is selected, running the N process version of a benchmark\n\
  on NP overall, means running on (NP/N) simultaneous groups of N each.\n\
\n\
  MultiMode only controls default (0) or extensive (1) output charts.\n\
  0: Only lowest performance groups is output\n\
  1: All groups are output\n\
\n\
  Default: multi off\n\
\n\
- Lengths_file is an ASCII file, containing any set of nonnegative\n\
  message lengths, 1 per line\n\
\n\
  Default: no Lengths_file, lengths defined by settings.h, settings_io.h\n\
  \n\
- filename is any text file containing, line by line, benchmark names.\n\
  Facilitates running particular benchmarks as compared to using the\n\
  command line.\n\
\n\
  Default: no input file exists\n\
  \n\
- benchmarkX is (in arbitrary lower/upper case spelling)\n\
\n");
#ifdef MPI1

fprintf(unit,"\
PingPong\n\
PingPing\n\
Sendrecv\n\
Exchange\n\
Bcast\n\
Allgather\n\
Allgatherv\n\
Alltoall\n\
Alltoallv\n\
Reduce\n\
Reduce_scatter\n\
Allreduce\n\
Barrier\n\
\n");

#elif defined(EXT)

fprintf(unit,"\
Window\n\
Unidir_Put\n\
Unidir_Get\n\
Bidir_Get\n\
Bidir_Put\n\
Accumulate\n\
\n");

#else

fprintf(unit,"\
S_Write_indv\n\
S_Read_indv\n\
S_Write_expl\n\
S_Read_expl\n\
P_Write_indv\n\
P_Read_indv\n\
P_Write_expl\n\
P_Read_expl\n\
P_Write_shared\n\
P_Read_shared\n\
P_Write_priv\n\
P_Read_priv\n\
C_Write_indv\n\
C_Read_indv\n\
C_Write_expl\n\
C_Read_expl\n\
C_Write_shared\n\
C_Read_shared\n\
\n");

#endif

}
