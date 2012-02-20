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

static inline int cpuid_ecx(int op)
{
#    ifdef _MSC_VER
     int result;
     _asm {
	  push ebx
          mov eax,op
          cpuid
          mov result,ecx
          pop ebx
     }
     return result;
#    else
     int eax, ecx, edx;

     __asm__("push %%ebx\n\tcpuid\n\tpop %%ebx"
	     : "=a" (eax), "=c" (ecx), "=d" (edx)
	     : "a" (op));
     return ecx;
#    endif
}

static inline int xgetbv_eax(int op)
{
#    ifdef _MSC_VER
     int veax, vedx;
     _asm {
          mov ecx,op
#    if defined(__INTEL_COMPILER) || (_MSC_VER >= 1600)
          xgetbv
#    else
          __emit 15
          __emit 1
          __emit 208
#    endif
          mov veax,eax
          mov vedx,edx
     }
     return veax;
#    else
     int eax, edx;
     __asm__ (".byte 0x0f, 0x01, 0xd0" : "=a"(eax), "=d"(edx) : "c" (op));
     return eax;
#endif
}
