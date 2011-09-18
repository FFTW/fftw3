/*
 * Copyright (c) 2003, 2007-11 Matteo Frigo
 * Copyright (c) 2003, 2007-11 Massachusetts Institute of Technology
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#ifdef _MSC_VER
#include <intrin.h>
#include <immintrin.h>
#ifndef inline
#define inline __inline
#endif
#endif

static inline int cpuid_ecx(int op)
{
#    ifdef _MSC_VER
     int info[4];
     __cpuid(info, op);
     return info[2];
#    else
     int eax, ecx, edx;

     __asm__("pushq %%rbx\n\tcpuid\n\tpopq %%rbx"
	     : "=a" (eax), "=c" (ecx), "=d" (edx)
	     : "a" (op));
     return ecx;
#    endif
}

static inline int xgetbv_eax(int op)
{
#    ifdef _MSC_VER
     /* I think _xgetbv() was introduced in "Service Pack 1"
	of "Visual Studio 2010".  It is unclear whether the
	spelling is _xgetbv() or __xgetbv().  If you cannot
	compile this function, send a note to fftw@fftw.org */
     return (int)(_xgetbv(op) & 0xFFFFFFFFLL);
#    else
     int eax, edx;
     __asm__ (".byte 0x0f, 0x01, 0xd0" : "=a"(eax), "=d"(edx) : "c" (op));
     return eax;
#endif
}
