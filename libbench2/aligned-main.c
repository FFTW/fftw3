/*
 * Copyright (c) 2001 Matteo Frigo
 * Copyright (c) 2001 Massachusetts Institute of Technology
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

/* $Id: aligned-main.c,v 1.7 2006-01-05 03:04:27 stevenj Exp $ */

#include "bench.h"

int aligned_main(int argc, char *argv[])
{
#if defined(__GNUC__) && defined(__i386__)
     /*
      * horrible hack to align the stack to a 16-byte boundary.
      *
      * We assume a gcc version >= 2.95 so that
      * -mpreferred-stack-boundary works.  Otherwise, all bets are
      * off.  However, -mpreferred-stack-boundary does not create a
      * stack alignment, but it only preserves it.  Unfortunately,
      * many versions of libc on linux call main() with the wrong
      * initial stack alignment, with the result that the code is now
      * pessimally aligned instead of having a 50% chance of being
      * correct.
      */
     {
	  /*
	   * Use alloca to allocate some memory on the stack.
	   * This alerts gcc that something funny is going
	   * on, so that it does not omit the frame pointer
	   * etc.
	   */
	  (void)__builtin_alloca(16); 

	  /*
	   * Now align the stack pointer
	   */
	  __asm__ __volatile__ ("andl $-16, %esp");

#  ifdef FFTW_DEBUG_ALIGNMENT
	  /* pessimally align the stack, in order to check whether the
	     stack re-alignment hacks in FFTW3 work */
	  __asm__ __volatile__ ("addl $-4, %esp");
#  endif
     }
#endif

#ifdef __ICC /* Intel's compiler for ia32 */
     {
	  /*
	   * Simply calling alloca seems to do the right thing. 
	   * The size of the allocated block seems to be irrelevant.
	   */
	  _alloca(16);
     }
#endif

     return bench_main(argc, argv);
}
