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

/* $Id: altivec.c,v 1.11 2005-02-13 23:17:32 athena Exp $ */

#include "ifftw.h"
#include "simd.h"

#if HAVE_ALTIVEC
const vector unsigned int X(altivec_constants)[] = {
     /* select even */
     /* 0: */ VLIT(0x00010203, 0x04050607, 0x10111213, 0x14151617),

     /* select odd */
     /* 1: */ VLIT(0x08090a0b, 0x0c0d0e0f, 0x18191a1b, 0x1c1d1e1f),

     /* swap real/imag */
     /* 2: */ VLIT(0x04050607, 0x00010203, 0x0c0d0e0f, 0x08090a0b),

     /* used in LD for alignment */
     /* 3: */ VLIT(0, 0, 0xFFFFFFFF, 0xFFFFFFFF),

     /* representation of (-1.0, 1.0, -1.0, 1.0).  Multiply by this
      * constant to change the sign of the real part */
     /* 4: */ VLIT(0xbf800000, 0x3f800000, 0xbf800000, 0x3f800000),

     /* xor with this to change the sign of the real part */
     /* 5: */ VLIT(0x80000000, 0x00000000, 0x80000000, 0x00000000)
};

#if HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif

#if HAVE_SYS_SYSCTL_H && HAVE_SYSCTL && defined(CTL_HW) && defined(HW_VECTORUNIT)
/* code for darwin */
static int really_have_altivec(void)
{
     int mib[2], altivecp;
     size_t len;
     mib[0] = CTL_HW;
     mib[1] = HW_VECTORUNIT;
     len = sizeof(altivecp);
     sysctl(mib, 2, &altivecp, &len, NULL, 0);
     return altivecp;
} 
#else /* HAVE_SYS_SYSCTL_H etc. */

#include <signal.h>
#include <setjmp.h>

static jmp_buf jb;
static vector unsigned int dummy;

static void sighandler(int x)
{
     longjmp(jb, 1);
}

static int really_have_altivec(void)
{
     void (*oldsig)(int);
     oldsig = signal(SIGILL, sighandler);
     if (setjmp(jb)) {
	  signal(SIGILL, oldsig);
	  return 0;
     } else {
	  dummy = vec_add(dummy, dummy);
	  signal(SIGILL, oldsig);
	  return 1;
     }
     return 0;
}
#endif

int RIGHT_CPU(void)
{
     static int init = 0, res;
     if (!init) {
	  res = really_have_altivec();
	  init = 1;
     }
     return res;
}
#endif
