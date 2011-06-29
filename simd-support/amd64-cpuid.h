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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#ifdef _MSC_VER
#ifndef inline
#define inline __inline
#endif
#endif

static inline int cpuid_ecx(int op)
{
#    ifdef _MSC_VER
#    error "Please write this code and send it to fftw@fftw.org"
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
#    error "Please write this code and send it to fftw@fftw.org"
#    else
     int eax, edx;
     __asm__ (".byte 0x0f, 0x01, 0xd0" : "=a"(eax), "=d"(edx) : "c" (op));
     return eax;
#endif
}
