/*
 * Copyright (c) 2003, 2007-8 Matteo Frigo
 * Copyright (c) 2003, 2007-8 Massachusetts Institute of Technology
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


/* this code was kindly donated by Eric J. Korpela */

#ifdef _MSC_VER
#ifndef inline
#define inline __inline
#endif
#endif

static inline int is_386() 
{
#ifdef _MSC_VER
    unsigned int result,tst;
    _asm {
        pushfd
        pop eax
        mov edx,eax
        xor eax,40000h
        push eax
        popfd
        pushfd
        pop eax
        push edx
        popfd
        mov tst,edx
        mov result,eax
    }
#else
    register unsigned int result,tst;
    __asm__ (
        "pushfl\n\t"
        "popl %0\n\t"
        "movl %0,%1\n\t"
        "xorl $0x40000,%0\n\t"
        "pushl %0\n\t"
        "popfl\n\t"
        "pushfl\n\t"
        "popl %0\n\t"
        "pushl %1\n\t"
        "popfl"
    : "=r" (result), "=r" (tst) /* output */
    :  /* no inputs */
    );
#endif
    return (result == tst);
}

static inline int has_cpuid() 
{
#ifdef _MSC_VER
    unsigned int result,tst;
    _asm {
        pushfd
        pop eax
        mov edx,eax
        xor eax,200000h
        push eax
        popfd
        pushfd
        pop eax
        push edx
        popfd
        mov tst,edx
        mov result,eax
    }
#else
    register unsigned int result,tst;
    __asm__ (
        "pushfl\n\t"
        "pop %0\n\t"
        "movl %0,%1\n\t"
        "xorl $0x200000,%0\n\t"
        "pushl %0\n\t"
        "popfl\n\t"
        "pushfl\n\t"
        "popl %0\n\t"
        "pushl %1\n\t"
        "popfl"
    : "=r" (result), "=r" (tst) /* output */
    : /* no inputs */
    );
#endif
    return (result != tst);
}

static inline int cpuid_edx(int op)
{
#    ifdef _MSC_VER
     int result;
     _asm {
	  push ebx
          mov eax,op
          cpuid
          mov result,edx
          pop ebx
     }
     return result;
#    else
     int eax, ecx, edx;

     __asm__("push %%ebx\n\tcpuid\n\tpop %%ebx"
	     : "=a" (eax), "=c" (ecx), "=d" (edx)
	     : "a" (op));
     return edx;
#    endif
}
