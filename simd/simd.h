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

#ifdef HAVE_SSE
#include "simd-sse.h"
#endif

#ifdef HAVE_SSE2
#include "simd-sse2.h"
#endif

#ifdef HAVE_ALTIVEC
#include "simd-altivec.h"
#endif

#if defined(HAVE_CELL) && !defined(ALIGNMENT)
/* align Cell data in the same way as Altivec */
#define ALIGNMENT 8 
#define ALIGNMENTA 16
#endif

#ifdef HAVE_MIPS_PS
#include "simd-mips_ps.h"
#endif

/* TAINT_BIT is set if pointers are not guaranteed to be multiples of
   ALIGNMENT */
#define TAINT_BIT 1    

/* TAINT_BITA is set if pointers are not guaranteed to be multiples of
   ALIGNMENTA */
#define TAINT_BITA 2

#define PTRINT(p) ((uintptr_t)(p))

#define ALIGNED(p) \
  (((PTRINT(UNTAINT(p)) % ALIGNMENT) == 0) && !(PTRINT(p) & TAINT_BIT))

#define ALIGNEDA(p) \
  (((PTRINT(UNTAINT(p)) % ALIGNMENTA) == 0) && !(PTRINT(p) & TAINT_BITA))

#define SIMD_STRIDE_OK(x) (!(((x) * sizeof(R)) % ALIGNMENT))
#define SIMD_STRIDE_OKA(x) (!(((x) * sizeof(R)) % ALIGNMENTA))
#define SIMD_VSTRIDE_OK SIMD_STRIDE_OK

