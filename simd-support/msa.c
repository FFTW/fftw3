/*
 * Copyright (c) 2003, 2007-14 Matteo Frigo
 * Copyright (c) 2003, 2007-14 Massachusetts Institute of Technology
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


#include "kernel/ifftw.h"

#if HAVE_MSA

/* check for an environment where signals are known to work */
#if defined(unix) || defined(linux)
  # include <signal.h>
  # include <setjmp.h>

  static jmp_buf jb;

  static void sighandler(int x)
  {
    UNUSED(x);
    longjmp(jb, 1);
  }

  static int msa_works(void)
  {
    void (*oldsig)(int);
    oldsig = signal(SIGILL, sighandler);
    if (setjmp(jb)) {
      signal(SIGILL, oldsig);
      return 0;
    } else {
      /* asm volatile ("xor.v $w0, $w0, $w0"); */
      asm volatile (".long 0x7860001e");
      signal(SIGILL, oldsig);
      return 1;
    }
  }

  int X(have_simd_msa)(void)
  {
    static int init = 0, res;

    if (!init) {
      res = msa_works();
      init = 1;
    }
    return res;
  }

#else
/* don't know how to autodetect MSA; assume it is present */
  int X(have_simd_msa)(void)
  {
    return 1;
  }
#endif

#endif
