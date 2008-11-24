/******************************************************************************\
*                                                                              *
*        Copyright (c) 2003, The Regents of the University of California       *
*      See the file COPYRIGHT for a complete copyright notice and license.     *
*                                                                              *
********************************************************************************
*
* CVS info:
*   $RCSfile: aiori-noHDF5.c,v $
*   $Revision: 1.7 $
*   $Date: 2006/07/12 00:04:47 $
*   $Author: loewe $
*
* Purpose:
*       Empty HDF5 functions for when compiling without HDF5 support.
*
\******************************************************************************/

#include "aiori.h"

void *
IOR_Create_HDF5(char        * testFileName,
		IOR_param_t * param)
{
    ERR("This copy of IOR was not compiled with HDF5 support");
    return 0;
}

void *
IOR_Open_HDF5(char        * testFileName,
              IOR_param_t * param)
{
    ERR("This copy of IOR was not compiled with HDF5 support");
    return 0;
}


IOR_offset_t
IOR_Xfer_HDF5(int            access,
              void         * fd,
              IOR_size_t   * buffer,
              IOR_offset_t   length,
              IOR_param_t  * param)
{
    ERR("This copy of IOR was not compiled with HDF5 support");
    return 0;
}

void
IOR_Fsync_HDF5(void * fd)
{
    ERR("This copy of IOR was not compiled with HDF5 support");
}

void
IOR_Close_HDF5(void        * fd,
               IOR_param_t * param)
{
    ERR("This copy of IOR was not compiled with HDF5 support");
}

void
IOR_Delete_HDF5(char * testFileName)
{
    ERR("This copy of IOR was not compiled with HDF5 support");
}

void
IOR_SetVersion_HDF5(IOR_param_t *test)
{
    ERR("This copy of IOR was not compiled with HDF5 support");
}

IOR_offset_t
IOR_GetFileSize_HDF5(IOR_param_t * test,
                     MPI_Comm   testComm,
                     char     * testFileName)
{
    ERR("This copy of IOR was not compiled with HDF5 support");
    return 0;
}
