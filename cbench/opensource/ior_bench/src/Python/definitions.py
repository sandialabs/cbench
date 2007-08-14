#!/usr/local/bin/python
# definitions for GUI
#
#/*****************************************************************************\
#*                                                                             *
#*       Copyright (c) 2003, The Regents of the University of California       *
#*     See the file COPYRIGHT for a complete copyright notice and license.     *
#*                                                                             *
#\*****************************************************************************/
#
# CVS info:
#   $RCSfile: definitions.py,v $
#   $Revision: 1.3 $
#   $Date: 2006/07/12 00:04:47 $
#   $Author: loewe $

# definitions
activeColor = "#F060F0"
bgColor = "cornsilk"
buttonColor = "lightBlue"
buttonTextColor = "#00008B"
fieldColor = "#EFEFFF"
smallFont = ("Arial", 10, "bold")
labelFont = ("Arial", 14, "bold")
titleFont = ("Arial", 18, "bold")
KIBIBYTE = 1024
MEBIBYTE = KIBIBYTE * KIBIBYTE
GIBIBYTE = KIBIBYTE * MEBIBYTE
IOR_SIZE_T = 8		# this is sizeof(IOR_size_t) from aiori.h,
			# necessary for the granularity of a transfer
WC_OL_THRESHOLD = 5	# this is the outlier threshold in seconds
