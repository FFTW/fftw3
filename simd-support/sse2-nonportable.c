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

#include "ifftw.h"

#if HAVE_SSE2
/* this file must be compiled with -msse/-msse2 or equivalent, and it will
   fail at runtime on a machine that does not support sse/sse2 */
#include "simd-sse2.h"

/* This will produce -0.0f (or -0.0d) even on broken
   compilers that do not distinguish +0.0 from -0.0.
   I bet some are still around. */
const union uvec X(sse2_pm) = {
#ifdef FFTW_SINGLE
     { 0x00000000, 0x80000000, 0x00000000, 0x80000000 }
#else
     { 0x00000000, 0x00000000, 0x00000000, 0x80000000 }
#endif
};

/* paranoia because of past compiler bugs */
void X(check_alignment_of_sse2_pm)(void)
{
     CK(ALIGNED(&X(sse2_pm)));
}
#endif
