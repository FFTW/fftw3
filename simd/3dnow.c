/*
 * Copyright (c) 2003 Matteo Frigo
 * Copyright (c) 2003 Massachusetts Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* $Id: 3dnow.c,v 1.3 2003-03-15 20:29:43 stevenj Exp $ */

#include "ifftw.h"
#include "simd.h"

#if HAVE_3DNOW

static inline int cpuid_edx(int op)
{
     int eax, ecx, edx;

     __asm__("push %%ebx\n\tcpuid\n\tpop %%ebx"
	     : "=a" (eax), "=c" (ecx), "=d" (edx)
	     : "a" (op));
     return edx;
}

static inline int cpuid_eax(int op)
{
     int eax, ecx, edx;

     __asm__("push %%ebx\n\tcpuid\n\tpop %%ebx"
	     : "=a" (eax), "=c" (ecx), "=d" (edx)
	     : "a" (op));
     return eax;
}

int RIGHT_CPU(void)
{
     static int init = 0, res;

     if (!init) {
	  init = 1;
	  res = 0;

	  if (cpuid_eax(0x80000000) >= 0x80000001) 
	       res =  (cpuid_edx(0x80000001) >> 31)
		    & (cpuid_edx(0x80000001) >> 30) 
		    & 1;
     }
     return res;
}

const union fvec X(3dnow_mp) = { {-0.0, 0.0} };

#endif
