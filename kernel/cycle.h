/*
 * Copyright (c) 2002 Matteo Frigo
 * Copyright (c) 2002 Steven G. Johnson
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

/* $Id: cycle.h,v 1.1 2002-06-03 12:09:05 athena Exp $ */

/* machine-dependent cycle counters code. Needs to be inlined. */

#if defined(HAVE_GETHRTIME)
typedef hrtime_t ticks;

#  define getticks() gethrtime()

static inline double elapsed(ticks t1, ticks t0)
{
     return (double)(t1 - t0);
}

#define HAVE_TICK_COUNTER
#endif


/*
 * PowerPC ``cycle'' counter using the time base register.
 */
#if defined(__GNUC__) && (defined(__powerpc__) || defined(__ppc__))
typedef unsigned long long ticks;

static __inline__ ticks getticks(void)
{
     unsigned int tbl, tbu0, tbu1;

     do {
	  __asm__ __volatile__ ("mftbu %0" : "=r"(tbu0));	
	  __asm__ __volatile__ ("mftb %0" : "=r"(tbl));
	  __asm__ __volatile__ ("mftbu %0" : "=r"(tbu1));	
     } while (tbu0 != tbu1);

     return (((unsigned long long)tbu0) << 32) | tbl;
}

static __inline__ double elapsed(ticks t1, ticks t0)
{
     return (double)(t1 - t0);
}

#define HAVE_TICK_COUNTER
#endif

/*
 * Pentium cycle counter 
 */
#if defined(__i386__)
typedef unsigned long long ticks;

static __inline__ ticks getticks(void)
{
     ticks ret;

     __asm__ __volatile__("rdtsc": "=A" (ret)); 
     /* no input, nothing else clobbered */
     return ret;
}

static __inline__ double elapsed(ticks t1, ticks t0)
{
     return (double)(t1 - t0);
}

#define HAVE_TICK_COUNTER
#endif

/*
 * IA64 cycle counter
 */
#if defined(__GNUC__) && defined(__ia64__)
typedef unsigned long ticks;

static __inline__ ticks getticks(void)
{
     ticks ret;

     __asm__ __volatile__ ("mov %0=ar.itc" : "=r"(ret));
     return ret;
}

static __inline__ double elapsed(ticks t1, ticks t0)
{
     return (double)(t1 - t0);
}

#define HAVE_TICK_COUNTER
#endif

/*
 * PA-RISC cycle counter 
 */
#if defined(__hppa__) || defined(__hppa)
typedef unsigned long ticks;

#  ifdef __GNUC__
static __inline__ ticks getticks(void)
{
     ticks ret;

     __asm__ __volatile__("mfctl 16, %0": "=r" (ret)); 
     /* no input, nothing else clobbered */
     return ret;
}
#  else
#  include <machine/inline.h>
static inline ticks getticks(void)
{
     register ticks ret;
     _MFCTL(16, ret);
     return ret;
}
#  endif

static __inline__ double elapsed(ticks t1, ticks t0)
{
     return (double)(t1 - t0);
}

#define HAVE_TICK_COUNTER
#endif

#if defined(__GNUC__) && defined(__alpha__)
/* 
    * The 32-bit cycle counter on alpha overflows pretty quickly, 
    * unfortunately.  A 1GHz machine overflows in 4 seconds.
    */
typedef unsigned int ticks;

static __inline__ ticks getticks(void)
{
     unsigned long cc;
     __asm__ __volatile__ ("rpcc %0" : "=r"(cc));	
     return (cc & 0xFFFFFFFF);
}

static __inline__ double elapsed(ticks t1, ticks t0)
{
     return (double)(t1 - t0);
}

#define HAVE_TICK_COUNTER
#endif

#if defined(__DECC) && defined(__alpha) && defined(HAVE_C_ASM_H)
#  include <c_asm.h>
typedef unsigned int ticks;

static __inline__ ticks getticks(void)
{
     unsigned long cc;
     cc = asm("rpcc %v0");
     return (cc & 0xFFFFFFFF);
}

static __inline__ double elapsed(ticks t1, ticks t0)
{
     return (double)(t1 - t0);
}

#define HAVE_TICK_COUNTER
#endif

/* SGI/Irix */
#if defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_SGI_CYCLE)
typedef struct timespec ticks;

static __inline__ ticks getticks(void)
{
     struct timespec t;
     clock_gettime(CLOCK_SGI_CYCLE, &t);
     return t;
}

static __inline__ double elapsed(ticks t1, ticks t0)
{
     return (double)(t1.tv_sec - t0.tv_sec) * 1.0E9 +
	  (double)(t1.tv_nsec - t0.tv_nsec);
}
#define HAVE_TICK_COUNTER
#define TIME_MIN_TICK 10000.0
#endif

