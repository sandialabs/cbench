/* 
 * -- High Performance Computing Linpack Benchmark (HPL)                
 *    HPL - 1.0a - January 20, 2004                          
 *    Antoine P. Petitet                                                
 *    University of Tennessee, Knoxville                                
 *    Innovative Computing Laboratories                                 
 *    (C) Copyright 2000-2004 All Rights Reserved                       
 *                                                                      
 * -- Copyright notice and Licensing terms:                             
 *                                                                      
 * Redistribution  and  use in  source and binary forms, with or without
 * modification, are  permitted provided  that the following  conditions
 * are met:                                                             
 *                                                                      
 * 1. Redistributions  of  source  code  must retain the above copyright
 * notice, this list of conditions and the following disclaimer.        
 *                                                                      
 * 2. Redistributions in binary form must reproduce  the above copyright
 * notice, this list of conditions,  and the following disclaimer in the
 * documentation and/or other materials provided with the distribution. 
 *                                                                      
 * 3. All  advertising  materials  mentioning  features  or  use of this
 * software must display the following acknowledgement:                 
 * This  product  includes  software  developed  at  the  University  of
 * Tennessee, Knoxville, Innovative Computing Laboratories.             
 *                                                                      
 * 4. The name of the  University,  the name of the  Laboratory,  or the
 * names  of  its  contributors  may  not  be used to endorse or promote
 * products  derived   from   this  software  without  specific  written
 * permission.                                                          
 *                                                                      
 * -- Disclaimer:                                                       
 *                                                                      
 * THIS  SOFTWARE  IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,  INCLUDING,  BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE UNIVERSITY
 * OR  CONTRIBUTORS  BE  LIABLE FOR ANY  DIRECT,  INDIRECT,  INCIDENTAL,
 * SPECIAL,  EXEMPLARY,  OR  CONSEQUENTIAL DAMAGES  (INCLUDING,  BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA OR PROFITS; OR BUSINESS INTERRUPTION)  HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT,  STRICT LIABILITY,  OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 * ---------------------------------------------------------------------
 */ 
/*
 * Include files
 */
#include "hpl.h"

#ifdef ASYOUGO2
   double ASYOUGO_dgemm_flops;
   double ASYOUGO_dgemm_time;
   int ASYOUGO_ntimes;
#endif

#ifdef STDC_HEADERS
void HPL_pdgesvK2
(
   HPL_T_grid *                     GRID,
   HPL_T_palg *                     ALGO,
   HPL_T_pmat *                     A
)
#else
void HPL_pdgesvK2
( GRID, ALGO, A )
   HPL_T_grid *                     GRID;
   HPL_T_palg *                     ALGO;
   HPL_T_pmat *                     A;
#endif
{
/* 
 * Purpose
 * =======
 *
 * HPL_pdgesvK2 factors a N+1-by-N matrix using LU factorization with row
 * partial pivoting.  The main algorithm  is the "right looking" variant
 * with look-ahead.  The  lower  triangular factor is left unpivoted and
 * the pivots are not returned. The right hand side is the N+1 column of
 * the coefficient matrix.
 *
 * Arguments
 * =========
 *
 * GRID    (local input)                 HPL_T_grid *
 *         On entry,  GRID  points  to the data structure containing the
 *         process grid information.
 *
 * ALGO    (global input)                HPL_T_palg *
 *         On entry,  ALGO  points to  the data structure containing the
 *         algorithmic parameters.
 *
 * A       (local input/output)          HPL_T_pmat *
 *         On entry, A points to the data structure containing the local
 *         array information.
 *
 * ---------------------------------------------------------------------
 */ 
/*
 * .. Local Variables ..
 */
   HPL_T_panel                * p, * * panel = NULL;
   HPL_T_UPD_FUN              HPL_pdupdate; 
   int                        N, depth, icurcol=0, j, jb, jj=0, jstart,
                              k, mycol, n, nb, nn, npcol, nq,
                              tag=MSGID_BEGIN_FACT, test=HPL_KEEP_TESTING;
#ifdef ENDEARLY
  #ifndef ASYOUGO
  #define ASYOUGO
  #endif
#endif
#ifdef ASYOUGO
	#define dclock dsecnd_
   extern double dclock();
   double start_time= dclock(), dprint=.005;
   double asyoutimer, mflops, dtmp, dtmp1;
   int myrow=GRID->myrow;
#endif
/* ..
 * .. Executable Statements ..
 */
#ifdef ASYOUGO2
   ASYOUGO_dgemm_flops= 0.0;
   ASYOUGO_dgemm_time = 0.0;
   ASYOUGO_ntimes = 0;
#endif
   mycol = GRID->mycol; npcol        = GRID->npcol;
   depth = ALGO->depth; HPL_pdupdate = ALGO->upfun;
   N     = A->n;        nb           = A->nb;

   if( N <= 0 ) return;
/*
 * Allocate a panel list of length depth + 1 (depth >= 1)
 */
   panel = (HPL_T_panel **)malloc( (depth+1) * sizeof( HPL_T_panel *) );
   if( panel == NULL )
   { HPL_pabort( __LINE__, "HPL_pdgesvK2", "Memory allocation failed" ); }
/*
 * Create and initialize the first depth panels
 */
   nq = HPL_numroc( N+1, nb, nb, mycol, 0, npcol ); nn = N; jstart = 0;

   for( k = 0; k < depth; k++ )
   {
      jb = Mmin( nn, nb );
      HPL_pdpanel_new( GRID, ALGO, nn, nn+1, jb, A, jstart, jstart,
                       tag, &panel[k] );
      nn -= jb; jstart += jb;
      if( mycol == icurcol ) { jj += jb; nq -= jb; }
      icurcol = MModAdd1( icurcol, npcol );
      tag     = MNxtMgid( tag, MSGID_BEGIN_FACT, MSGID_END_FACT );
   }
/*
 * Create last depth+1 panel
 */
   HPL_pdpanel_new( GRID, ALGO, nn, nn+1, Mmin( nn, nb ), A, jstart,
                    jstart, tag, &panel[depth] );
   tag = MNxtMgid( tag, MSGID_BEGIN_FACT, MSGID_END_FACT );
/*
 * Initialize the lookahead - Factor jstart columns: panel[0..depth-1]
 */
   for( k = 0, j = 0; k < depth; k++ )
   {
      jb = jstart - j; jb = Mmin( jb, nb ); j += jb;
/*
 * Factor and broadcast k-th panel
 */
      HPL_pdfact(         panel[k] );
      (void) HPL_binit(   panel[k] );
      do
      { (void) HPL_bcast( panel[k], &test ); }
      while( test != HPL_SUCCESS );
      (void) HPL_bwait(   panel[k] );
/*
 * Partial update of the depth-k-1 panels in front of me
 */
      if( k < depth - 1 )
      {
         nn = HPL_numrocI( jstart-j, j, nb, nb, mycol, 0, npcol );
         HPL_pdupdate( NULL, NULL, panel[k], nn );
      }
   }
/*
 * Main loop over the remaining columns of A
 */
   for( j = jstart; j < N; j += nb )
   {
#ifdef ASYOUGO
      if ( j > dprint*N )
      {
         asyoutimer = dclock() - start_time;
         dtmp = (double) N;
         dtmp1 = (double)(N-j);
         mflops = 2.0*(dtmp*dtmp*dtmp-dtmp1*dtmp1*dtmp1)/3.0;
         mflops = mflops / (1000000.0*asyoutimer);
/*
#ifdef ASYOUGO2
printf("(%d,%d) Col=%06d start_time=%9.1f ourtime=%g\n", 
       myrow,mycol,j,start_time,asyoutimer);
#endif
*/
         if ( myrow==0 && mycol==0 )
#ifdef ASYOUGO2
            printf("Col=%06d Fract=%4.3f Mflops=%8.2f (DT=%9.1f DF=%9.1f DMF=%8.2f)\n",j,dprint,mflops,ASYOUGO_dgemm_time,ASYOUGO_dgemm_flops/1000000000.0,GRID->nprow*GRID->npcol*ASYOUGO_dgemm_flops/(1000000.0*asyoutimer));
#else
            printf("Column=%06d Fraction=%4.3f Mflops=%8.2f Elapsed=%0.2f min\n",j,dprint,mflops,asyoutimer/60.0);
#endif
         fflush(NULL);
         if ( dprint < .195 ) dprint += 0.005;
         else dprint += 0.1;
      }
#endif
#ifdef ENDEARLY
      if ( dprint >= .04 ) 
      {
         A->info = j;
         return ;
      }
#endif
 
      n = N - j; jb = Mmin( n, nb );
/*
 * Initialize current panel - Finish latest update, Factor and broadcast
 * current panel
 */
      (void) HPL_pdpanel_free( panel[depth] );
      HPL_pdpanel_init( GRID, ALGO, n, n+1, jb, A, j, j, tag, panel[depth] );

      if( mycol == icurcol )
      {
         nn = HPL_numrocI( jb, j, nb, nb, mycol, 0, npcol );
         for( k = 0; k < depth; k++ )   /* partial updates 0..depth-1 */
            (void) HPL_pdupdate( NULL, NULL, panel[k], nn );
         HPL_pdfact(       panel[depth] );    /* factor current panel */
      }
      else { nn = 0; }
          /* Finish the latest update and broadcast the current panel */
      (void) HPL_binit( panel[depth] );
      HPL_pdupdate( panel[depth], &test, panel[0], nq-nn );
      (void) HPL_bwait( panel[depth] );
/*
 * Circular  of the panel pointers:
 * xtmp = x[0]; for( k=0; k < depth; k++ ) x[k] = x[k+1]; x[d] = xtmp;
 *
 * Go to next process row and column - update the message ids for broadcast
 */
      p = panel[0]; for( k = 0; k < depth; k++ ) panel[k] = panel[k+1];
      panel[depth] = p;

      if( mycol == icurcol ) { jj += jb; nq -= jb; }
      icurcol = MModAdd1( icurcol, npcol );
      tag     = MNxtMgid( tag, MSGID_BEGIN_FACT, MSGID_END_FACT );
   }
/*
 * Clean-up: Finish updates - release panels and panel list
 */
   nn = HPL_numrocI( 1, N, nb, nb, mycol, 0, npcol );
   for( k = 0; k < depth; k++ )
   {
      (void) HPL_pdupdate( NULL, NULL, panel[k], nn );
      (void) HPL_pdpanel_disp(  &panel[k] );
   }
   (void) HPL_pdpanel_disp( &panel[depth] );

   if( panel ) free( panel );
/*
 * End of HPL_pdgesvK2
 */
}
