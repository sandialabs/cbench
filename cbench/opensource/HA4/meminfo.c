/*
###############################################################################
#    Copyright (2005) Sandia Corporation.  Under the terms of Contract
#    DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
#    certain rights in this software
#
#    This file is part of Cbench.
#
#    Cbench is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    any later version.
#
#    Cbench is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with Cbench; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
###############################################################################
*/

#include <stdio.h>
#include <string.h>

/* Returns total memory in kB in first param and swap used in kB in second param.
 * For 2.4 kernels only (meminfo changed format in 2.5)
 */
int meminfo(unsigned long long *mem, unsigned long long *swap_used) {
        FILE *meminfo;
        char label[100];
        unsigned long long data1, data2;
        int mem_found = 0;
        int swap_found = 0;

        if ( ( meminfo = fopen("/proc/meminfo", "r") ) == NULL ) {
                perror("fopen");
                return -1;
        }

        while (
                !(mem_found && swap_found) &&
                (fscanf(meminfo, "%5s %llu %llu", label, &data1, &data2) >= 1)
              ) {
                if ( ! strncmp(label, "Mem:", 4) ) {
                        //printf("DEBUG: mem found  (%s %llu %llu)\n", label, data1, data2);
                        *mem = data1 >> 10;
                        mem_found++;
                } else if ( ! strncmp(label, "Swap:", 5) ) {
                        //printf("DEBUG: swap found (%s %llu %llu)\n", label, data1, data2);
                        *swap_used = data2 >> 10;
                        swap_found++;
                }
        }

        fclose(meminfo);
        //printf("DEBUG: mem_found = %d\tswap_found = %d\n", mem_found, swap_found);

        if ( !mem_found || !swap_found )
            return -2;

        return 0;
}
