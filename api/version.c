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


#include "api.h"

const char X(cc)[] = FFTW_CC;

/* fftw <= 3.2.2 had special compiler flags for codelets, which are
   not used anymore.  We keep this variable around because it is part
   of the ABI */
const char X(codelet_optim)[] = "";

const char X(version)[] = PACKAGE "-" PACKAGE_VERSION

#if HAVE_FMA
   "-fma"
#endif

#if HAVE_SSE2
   "-sse2"
#endif

#if HAVE_AVX
   "-avx"
#endif

#if HAVE_ALTIVEC
   "-altivec"
#endif

#if HAVE_NEON
   "-neon"
#endif

;
