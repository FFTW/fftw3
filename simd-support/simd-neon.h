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

#ifndef FFTW_SINGLE
#error "NEON only works in single precision"
#endif

/* define these unconditionally, because they are used by
   taint.c which is compiled without neon */
#define SIMD_SUFFIX _neon	/* for renaming */
#define VL 1			/* SIMD complex vector length */
#define SIMD_VSTRIDE_OKA(x) 1
#define SIMD_STRIDE_OKPAIR SIMD_STRIDE_OKA

#if defined(__GNUC__) && !defined(__ARM_NEON__)
#error "compiling simd-neon.h requires -mfpu=neon or equivalent"
#endif

#include <arm_neon.h>

/* FIXME: I am not sure whether this code assumes little-endian
   ordering.  VLIT may or may not be wrong for big-endian systems. */
typedef float32x2_t V;
#define VLIT(x0, x1) {x0, x1}
#define LDK(x) x
#define DVK(var, val) const V var = VLIT(val, val)

/* NEON has FMA, but a three-operand FMA is not too useful
   for FFT purposes.  We normally compute

      t0=a+b*c
      t1=a-b*c

   In a three-operand instruction set this translates into

      t0=a
      t0+=b*c
      t1=a
      t1-=b*c

   At least one move must be implemented, negating the advantage of
   the FMA in the first place.  At least some versions of gcc generate
   both moves.  So we are better off generating t=b*c;t0=a+t;t1=a-t;*/
#if HAVE_FMA
#warning "--enable-fma on NEON is probably a bad idea (see source code)"
#endif

#define VADD(a, b) vadd_f32(a, b)
#define VSUB(a, b) vsub_f32(a, b)
#define VMUL(a, b) vmul_f32(a, b)
#define VFMA(a, b, c) vmla_f32(c, a, b)	        /* a*b+c */
#define VFNMS(a, b, c) vmls_f32(c, a, b)	/* FNMS=-(a*b-c) in powerpc terminology; MLS=c-a*b
						   in ARM terminology */
#define VFMS(a, b, c) VSUB(VMUL(a, b), c)	/* FMS=a*b-c in powerpc terminology; no equivalent
						   arm instruction */

#define LDA(x, ivs, aligned_like) vld1_f32((const float32_t *)x)
#define LD LDA

#define STA(x, v, ovs, aligned_like) vst1_f32((float32_t *)x, v)
#define ST STA

/* store and 2x2 complex transpose */
#define STM2 STA
#define STN2(x, v0, v1, ovs)	/* using the STM2 form */

/* store and 2x2 real transpose */
static inline void STM4(R *x, V v, INT ovs, const R *aligned_like)
{
     (void) aligned_like;	/* UNUSED */
     vst1_lane_f32((float32_t *)(x)      , v, 0);
     vst1_lane_f32((float32_t *)(x + ovs), v, 1);
}

#define STN4(x, v0, v1, v2, v3, ovs)	/* using the STM4 form */

#define FLIP_RI(x) vrev64_f32(x)

static inline V VCONJ(V x)
{
     /* FIXME: there ought to be a way to XOR floating-point values */
     const V pm = VLIT(1.0, -1.0);
     return VMUL(x, pm);
}

static inline V VBYI(V x)
{
     return FLIP_RI(VCONJ(x));
}

static inline V VFMAI(V b, V c)
{
     const V mp = VLIT(-1.0, 1.0);
     return VFMA(FLIP_RI(b), mp, c);
}

static inline V VFNMSI(V b, V c)
{
     const V mp = VLIT(-1.0, 1.0);
     return VFNMS(FLIP_RI(b), mp, c);
}

static inline V VFMACONJ(V b, V c)
{
     const V pm = VLIT(1.0, -1.0);
     return VFMA(b, pm, c);
}

static inline V VFNMSCONJ(V b, V c)
{
     const V pm = VLIT(1.0, -1.0);
     return VFNMS(b, pm, c);
}

static inline V VFMSCONJ(V b, V c)
{
     return VSUB(VCONJ(b), c);
}

#define VDUPL(x) vdup_lane_f32(x, 0)
#define VDUPH(x) vdup_lane_f32(x, 1)

static inline V VZMUL(V tx, V sr)
{
     V tr = VDUPL(tx);
     V ti = VDUPH(tx);
     tr = VMUL(sr, tr);
     sr = VBYI(sr);
     return VFMA(ti, sr, tr);
}

static inline V VZMULJ(V tx, V sr)
{
     V tr = VDUPL(tx);
     V ti = VDUPH(tx);
     tr = VMUL(sr, tr);
     sr = VBYI(sr);
     return VFNMS(ti, sr, tr);
}

static inline V VZMULI(V tx, V sr)
{
     V tr = VDUPL(tx);
     V ti = VDUPH(tx);
     ti = VMUL(ti, sr);
     sr = VBYI(sr);
     return VFMS(tr, sr, ti);
}

static inline V VZMULIJ(V tx, V sr)
{
     V tr = VDUPL(tx);
     V ti = VDUPH(tx);
     ti = VMUL(ti, sr);
     sr = VBYI(sr);
     return VFMA(tr, sr, ti);
}

/* twiddle storage #1: compact, slower */
#define VTW1(v,x) {TW_CEXP, v, x}
#define TWVL1 1
static inline V BYTW1(const R *t, V sr)
{
     V tx = LD(t, 1, t);
     return VZMUL(tx, sr);
}

static inline V BYTWJ1(const R *t, V sr)
{
     V tx = LD(t, 1, t);
     return VZMULJ(tx, sr);
}

/* twiddle storage #2: twice the space, faster (when in cache) */
#define VTW2(v,x) {TW_COS, v, x}, {TW_COS, v, x}, {TW_SIN, v, -x}, {TW_SIN, v, x}
#define TWVL2 2

static inline V BYTW2(const R *t, V sr)
{
     const V *twp = (const V *) t;
     V si = FLIP_RI(sr);
     V tr = twp[0], ti = twp[1];
     return VFMA(ti, si, VMUL(tr, sr));
}

static inline V BYTWJ2(const R *t, V sr)
{
     const V *twp = (const V *) t;
     V si = FLIP_RI(sr);
     V tr = twp[0], ti = twp[1];
     return VFNMS(ti, si, VMUL(tr, sr));
}

/* twiddle storage #3 */
#define VTW3(v,x) {TW_CEXP, v, x}
#define TWVL3 1

/* twiddle storage for split arrays */
#define VTWS(v,x) {TW_COS, v, x}, {TW_COS, v+1, x}, {TW_SIN, v, x}, {TW_SIN, v+1, x}
#define TWVLS 2

#define VLEAVE()		/* nothing */

#include "simd-common.h"
