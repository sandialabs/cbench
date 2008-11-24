/******************************************************************************\
*                                                                              *
*        Copyright (c) 2003, The Regents of the University of California       *
*      See the file COPYRIGHT for a complete copyright notice and license.     *
*                                                                              *
********************************************************************************
*
* CVS info:
*   $RCSfile: aiori-NCMPI.c,v $
*   $Revision: 1.8 $
*   $Date: 2006/07/12 00:04:46 $
*   $Author: loewe $
*
* Purpose:
*       Implementation of abstract I/O interface for Parallel NetCDF (NCMPI).
*
\******************************************************************************/

#include "aiori.h"                        /* abstract IOR interface */
#include <errno.h>                        /* sys_errlist */
#include <stdio.h>                        /* only for fprintf() */
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pnetcdf.h>

#define NUM_DIMS 1                        /* number of dimensions to data set */

/******************************************************************************/
/*
 * NCMPI_CHECK will display a custom error message and then exit the program
 */

#define NCMPI_CHECK(NCMPI_RETURN, MSG) do {                              \
    char resultString[1024];                                             \
                                                                         \
    if (NCMPI_RETURN < 0) {                                              \
        fprintf(stdout, "** error **\n");                                \
        fprintf(stdout, "ERROR in %s (line %d): %s.\n",                  \
                __FILE__, __LINE__, MSG);                                \
        fprintf(stdout, "ERROR: %s.\n", ncmpi_strerror(NCMPI_RETURN));   \
        fprintf(stdout, "** exiting **\n");                              \
        exit(1);                                                         \
    }                                                                    \
} while(0)

/**************************** P R O T O T Y P E S *****************************/

int GetFileMode(IOR_param_t *);

/************************** D E C L A R A T I O N S ***************************/

extern int      errno,                                /* error number */
                rank,
                rankOffset,
                verbose;                              /* verbose output */
extern MPI_Comm testComm;

/***************************** F U N C T I O N S ******************************/
/******************************************************************************/
/*
 * Create and open a file through the NCMPI interface.
 */

void *
IOR_Create_NCMPI(char        * testFileName,
                 IOR_param_t * param)
{
    int * fd;
    int   fd_mode;

    fd = (int *)malloc(sizeof(int));
    if (fd == NULL) ERR("Unable to malloc file descriptor");

    fd_mode = GetFileMode(param);
    NCMPI_CHECK(ncmpi_create(testComm, testFileName, fd_mode,
                             MPI_INFO_NULL, fd), "cannot create file");
    return(fd);
} /* IOR_Create_NCMPI() */


/******************************************************************************/
/*
 * Open a file through the NCMPI interface.
 */

void *
IOR_Open_NCMPI(char        * testFileName,
               IOR_param_t * param)
{
    int * fd;
    int   fd_mode;

    fd = (int *)malloc(sizeof(int));
    if (fd == NULL) ERR("Unable to malloc file descriptor");

    fd_mode = GetFileMode(param);
    NCMPI_CHECK(ncmpi_open(testComm, testFileName, fd_mode,
                           MPI_INFO_NULL, fd), "cannot open file");
    return(fd);
} /* IOR_Open_NCMPI() */


/******************************************************************************/
/*
 * Write or read access to file using the NCMPI interface.
 */

IOR_offset_t
IOR_Xfer_NCMPI(int            access,
               void         * fd,
               IOR_size_t   * buffer,
               IOR_offset_t   length,
               IOR_param_t  * param)
{
    char         * bufferPtr          = (char *)buffer;
    static int     firstReadCheck     = FALSE,
                   startDataSet;
    int            data_id,
                   var_id,
                   dim_id[NUM_DIMS];
    size_t         fileSize;
    MPI_Offset     bufSize[NUM_DIMS],
                   offset[NUM_DIMS];
    IOR_offset_t   segmentPosition;

    bufSize[0] = (size_t)length;
    offset[0] = (size_t)(param->offset);

    /* determine by offset if need to start data set */
    if (param->filePerProc == TRUE) {
        segmentPosition = (IOR_offset_t)0;
    } else {
        segmentPosition = (IOR_offset_t)((rank + rankOffset) % param->numTasks)
                                        * param->blockSize;
    }
    if ((int)(param->offset - segmentPosition) == 0) {
        startDataSet = TRUE;
        /*
         * this toggle is for the read check operation, which passes through
         * this function twice; note that this function will open a data set
         * only on the first read check and close only on the second
         */
        if (access == READCHECK) {
            if (firstReadCheck == TRUE) {
                firstReadCheck = FALSE;
            } else {
                firstReadCheck = TRUE;
            }
        }
    }

    if (startDataSet == TRUE &&
        (access != READCHECK || firstReadCheck == TRUE)) {
        if (access == WRITE) {
            fileSize = param->blockSize * param->segmentCount;
            if (param->filePerProc == FALSE) {
                fileSize *= param->numTasks;
            }
    
            NCMPI_CHECK(ncmpi_def_dim(*(int *)fd, "data", fileSize, &data_id),
                        "cannot define data set dimensions");
            dim_id[0] = data_id;
            NCMPI_CHECK(ncmpi_def_var(*(int *)fd, "data_var", NC_BYTE,
                                      NUM_DIMS, dim_id, &var_id),
                        "cannot define data set variables");
            NCMPI_CHECK(ncmpi_enddef(*(int *)fd),
                        "cannot close data set define mode");
        
        } else {
            NCMPI_CHECK(ncmpi_inq_varid(*(int *)fd, "data_var", &var_id),
                        "cannot retrieve data set variable");
        }

        if (param->collective == FALSE) {
            NCMPI_CHECK(ncmpi_begin_indep_data(*(int *)fd),
                        "cannot enable independent data mode");
        }

        param->var_id = var_id;
        startDataSet = FALSE;
    }

    var_id = param->var_id;

    /* access the file */
    if (access == WRITE) { /* WRITE */
        if (param->collective) {
            NCMPI_CHECK(ncmpi_put_vara_all(*(int *)fd, var_id, offset, bufSize,
                                           bufferPtr, length, MPI_BYTE),
                        "cannot write to data set");
        } else {
            NCMPI_CHECK(ncmpi_put_vara(*(int *)fd, var_id, offset, bufSize,
                                       bufferPtr, length, MPI_BYTE),
                        "cannot write to data set");
        }
    } else {               /* READ or CHECK */
        if (param->collective == TRUE) {
            NCMPI_CHECK(ncmpi_get_vara_all(*(int *)fd, var_id, offset, bufSize,
                                           bufferPtr, length, MPI_BYTE),
                        "cannot read from data set");
        } else {
            NCMPI_CHECK(ncmpi_get_vara(*(int *)fd, var_id, offset, bufSize,
                                       bufferPtr, length, MPI_BYTE),
                        "cannot read from data set");
        }
    }

    return(length);
} /* IOR_Xfer_NCMPI() */


/******************************************************************************/
/*
 * Perform fsync().
 */

void
IOR_Fsync_NCMPI(void * fd)
{
    ;
} /* IOR_Fsync_NCMPI() */


/******************************************************************************/
/*
 * Close a file through the NCMPI interface.
 */

void
IOR_Close_NCMPI(void       * fd,
               IOR_param_t * param)
{
    if (param->collective == FALSE) {
        NCMPI_CHECK(ncmpi_end_indep_data(*(int *)fd),
                    "cannot disable independent data mode");
    }
    NCMPI_CHECK(ncmpi_close(*(int *)fd), "cannot close file");
    free(fd);
} /* IOR_Close_NCMPI() */


/******************************************************************************/
/*
 * Delete a file through the NCMPI interface.
 */

void
IOR_Delete_NCMPI(char * testFileName)
{
    if (unlink(testFileName) != 0) ERR("cannot delete file");
} /* IOR_Delete_NCMPI() */


/******************************************************************************/
/*
 * Determine api version.
 */

void
IOR_SetVersion_NCMPI(IOR_param_t * test)
{
    sprintf(test->apiVersion, "%s (%s)",
            test->api, ncmpi_inq_libvers());
} /* IOR_SetVersion_NCMPI() */


/************************ L O C A L   F U N C T I O N S ***********************/

/******************************************************************************/
/*
 * Return the correct file mode for NCMPI.
 */

int
GetFileMode(IOR_param_t * param)
{
    int fd_mode = 0;

    /* set IOR file flags to NCMPI flags */
    /* -- file open flags -- */
    if (param->openFlags & IOR_RDONLY) {fd_mode |= NC_NOWRITE;}
    if (param->openFlags & IOR_WRONLY) {
        fprintf(stdout, "File write only not implemented in NCMPI\n");
    }
    if (param->openFlags & IOR_RDWR)   {fd_mode |= NC_WRITE;}
    if (param->openFlags & IOR_APPEND) {
        fprintf(stdout, "File append not implemented in NCMPI\n");
    }
    if (param->openFlags & IOR_CREAT)  {fd_mode |= NC_CLOBBER;}
    if (param->openFlags & IOR_EXCL)   {
        fprintf(stdout, "Exclusive access not implemented in NCMPI\n");
    }
    if (param->openFlags & IOR_TRUNC)  {
        fprintf(stdout, "File truncation not implemented in NCMPI\n");
    }
    if (param->openFlags & IOR_DIRECT) {
        fprintf(stdout, "O_DIRECT not implemented in NCMPI\n");
    }
    return(fd_mode);
} /* GetFileMode() */


/******************************************************************************/
/*
 * Use MPIIO call to get file size.
 */

IOR_offset_t
IOR_GetFileSize_NCMPI(IOR_param_t * test,
                      MPI_Comm      testComm,
                      char        * testFileName)
{
    return(IOR_GetFileSize_NCMPI(test, testComm, testFileName));
} /* IOR_GetFileSize_NCMPI() */
