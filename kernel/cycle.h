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

/* $Id: cycle.h,v 1.27 2003-04-19 13:18:25 athena Exp $ */

/* machine-dependent cycle counters code. Needs to be inlined. */

/* This file uses macros like HAVE_GETHRTIME that are assumed to be
   defined according to whether the corresponding function/type/header
   is available on your system.  The necessary macros are most
   conveniently defined if you are using GNU autoconf, via the tests:
   
   dnl ---------------------------------------------------------------------

   AC_HEADER_TIME

   AC_CHECK_HEADERS([sys/time.h c_asm.h intrinsics.h])

   AC_CHECK_TYPE([hrtime_t],[AC_DEFINE(HAVE_HRTIME_T, 1, [Define to 1 if hrtime_t is defined in <sys/time.h>])],,[#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif])


   AC_CHECK_FUNCS([gethrtime read_real_time time_base_to_time clock_gettime])

   dnl Cray UNICOS _rtc() (real-time clock) intrinsic
   AC_MSG_CHECKING([for _rtc intrinsic])
   rtc_ok=yes
   AC_TRY_LINK([#ifdef HAVE_INTRINSICS_H
#include <intrinsics.h>
#endif], [_rtc()], [AC_DEFINE(HAVE__RTC,1,[Define if you have the UNICOS _rtc() intrinsic.])], [rtc_ok=no])
   AC_MSG_RESULT($rtc_ok)

   dnl ---------------------------------------------------------------------
*/

/*----------------------------------------------------------------*/
/* Solaris */
#if defined(HAVE_GETHRTIME) && defined(HAVE_HRTIME_T) && !defined(HAVE_TICK_COUNTER)
typedef hrtime_t ticks;

#define getticks gethrtime

static inline double elapsed(ticks t1, ticks t0)
{
     return (double)(t1 - t0);
}

#define HAVE_TICK_COUNTER
#endif

/*----------------------------------------------------------------*/
/* AIX v. 4+ routines to read the real-time clock or time-base register */
#if defined(HAVE_READ_REAL_TIME) && defined(HAVE_TIME_BASE_TO_TIME) && !defined(HAVE_TICK_COUNTER)
typedef timebasestruct_t ticks;

static inline ticks getticks(void)
{
     ticks t;
     read_real_time(&t, TIMEBASE_SZ);
     return t;
}

static inline double elapsed(ticks t1, ticks t0) /* time in nanoseconds */
{
     time_base_to_time(&t1, TIMEBASE_SZ);
     time_base_to_time(&t0, TIMEBASE_SZ);
     return ((t1.tb_high - t0.tb_high) * 1e9 + (t1.tb_low - t0.tb_low));
}

#define HAVE_TICK_COUNTER
#endif

/*----------------------------------------------------------------*/
/*
 * PowerPC ``cycle'' counter using the time base register.
 */
#if ((defined(__GNUC__) && (defined(__powerpc__) || defined(__ppc__))) || (defined(__MWERKS__) && defined(macintosh)))  && !defined(HAVE_TICK_COUNTER)
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
/*----------------------------------------------------------------*/
/*
 * Pentium cycle counter 
 */
#if (defined(__GNUC__) || defined(__ICC)) && defined(__i386__)  && !defined(HAVE_TICK_COUNTER)
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
/*----------------------------------------------------------------*/
/*
 * X86-64 cycle counter
 */
#if defined(__GNUC__) && defined(__x86_64__)  && !defined(HAVE_TICK_COUNTER)
typedef unsigned long long ticks;

static __inline__ ticks getticks(void)
{
     unsigned a, d; 
     asm volatile("rdtsc" : "=a" (a), "=d" (d)); 
     return ((ticks)a) | (((ticks)d) << 32); 
}

static __inline__ double elapsed(ticks t1, ticks t0)
{
     return (double)(t1 - t0);
}

#define HAVE_TICK_COUNTER
#endif
/*----------------------------------------------------------------*/
/*
 * IA64 cycle counter
 */
#if defined(__GNUC__) && defined(__ia64__) && !defined(HAVE_TICK_COUNTER)
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

/* HP/UX IA64 compiler, courtesy Teresa L. Johnson: */
#if defined(__hpux) && defined(__ia64) && !defined(HAVE_TICK_COUNTER)
#include <machine/sys/inline.h>
typedef unsigned long ticks;

static inline ticks getticks(void)
{
     ticks ret;

     ret = _Asm_mov_from_ar (_AREG_ITC);
     return ret;
}

static inline double elapsed(ticks t1, ticks t0)
{
     return (double)(t1 - t0);
}

#define HAVE_TICK_COUNTER
#endif

/* intel's ecc compiler */
#if defined(__ECC) && defined(__ia64__) && !defined(HAVE_TICK_COUNTER)
typedef unsigned long ticks;
#include <ia64intrin.h>

static __inline__ ticks getticks(void)
{
     return __getReg(_IA64_REG_AR_ITC);
}
 
static __inline__ double elapsed(ticks t1, ticks t0)
{
     return (double)(t1 - t0);
}
 
#define HAVE_TICK_COUNTER
#endif

/*----------------------------------------------------------------*/
/*
 * PA-RISC cycle counter 
 */
#if defined(__hppa__) || defined(__hppa) && !defined(HAVE_TICK_COUNTER)
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
static inline unsigned long getticks(void)
{
     register ticks ret;
     _MFCTL(16, ret);
     return ret;
}
#  endif

static inline double elapsed(ticks t1, ticks t0)
{
     return (double)(t1 - t0);
}

#define HAVE_TICK_COUNTER
#endif
/*----------------------------------------------------------------*/
#if defined(__GNUC__) && defined(__alpha__) && !defined(HAVE_TICK_COUNTER)
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

/*----------------------------------------------------------------*/
#if defined(__GNUC__) && defined(__sparc_v9__) && !defined(HAVE_TICK_COUNTER)
typedef unsigned long ticks;

static __inline__ ticks getticks(void)
{
     ticks ret;
     __asm__("rd %%tick, %0" : "=r" (ret));
     return ret;
}

static __inline__ double elapsed(ticks t1, ticks t0)
{
     return (double)(t1 - t0);
}

#define HAVE_TICK_COUNTER
#endif

/*----------------------------------------------------------------*/
#if defined(__DECC) && defined(__alpha) && defined(HAVE_C_ASM_H) && !defined(HAVE_TICK_COUNTER)
#  include <c_asm.h>
typedef unsigned int ticks;

static __inline ticks getticks(void)
{
     unsigned long cc;
     cc = asm("rpcc %v0");
     return (cc & 0xFFFFFFFF);
}

static __inline double elapsed(ticks t1, ticks t0)
{
     return (double)(t1 - t0);
}

#define HAVE_TICK_COUNTER
#endif
/*----------------------------------------------------------------*/
/* SGI/Irix */
#if defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_SGI_CYCLE) && !defined(HAVE_TICK_COUNTER)
typedef struct timespec ticks;

static inline ticks getticks(void)
{
     struct timespec t;
     clock_gettime(CLOCK_SGI_CYCLE, &t);
     return t;
}

static inline double elapsed(ticks t1, ticks t0)
{
     return (double)(t1.tv_sec - t0.tv_sec) * 1.0E9 +
	  (double)(t1.tv_nsec - t0.tv_nsec);
}
#define HAVE_TICK_COUNTER
#endif

/*----------------------------------------------------------------*/
/* Cray UNICOS _rtc() intrinsic function */
#if defined(HAVE__RTC) && !defined(HAVE_TICK_COUNTER)
#ifdef HAVE_INTRINSICS_H
#  include <intrinsics.h>
#endif

typedef long long ticks;

#define getticks _rtc

static inline double elapsed(ticks t1, ticks t0)
{
     return (double)(t1 - t0);
}

#define HAVE_TICK_COUNTER
#endif

