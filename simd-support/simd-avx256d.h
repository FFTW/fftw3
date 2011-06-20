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

#if defined(FFTW_SINGLE) || defined(FFTW_LDOUBLE) || defined(FFTW_QUAD)
#error "AVX256D only works in double precision"
#endif

#define SIMD_SUFFIX  _avx256d  /* for renaming */
#define VL 2            /* SIMD complex vector length */
#define SIMD_VSTRIDE_OKA(x) ((x) == 2)
#define SIMD_STRIDE_OKPAIR SIMD_STRIDE_OK

#if defined(__GNUC__) && !defined(__AVX__) /* sanity check */
#error "compiling simd-avx.h without -mavx"
#endif

#include <immintrin.h>

typedef __m256d V;
#define VADD _mm256_add_pd
#define VSUB _mm256_sub_pd
#define VMUL _mm256_mul_pd
#define VXOR _mm256_xor_pd
#define VSHUFPD _mm256_shuffle_pd
#define VDUPL(x) _mm256_unpacklo_pd(x, x)
#define VDUPH(x) _mm256_unpackhi_pd(x, x)

#define VLIT(x0, x1, x2, x3) _mm256_set_pd(x0, x1, x2, x3)
#define DVK(var, val) V var = VLIT(val, val, val, val)
#define LDK(x) x

static inline V LD(const R *x, INT ivs, const R *aligned_like)
{
     V var;
     (void)aligned_like; /* UNUSED */
     /*     var = _mm256_castpd128_pd256(*(const __m128d *)x); */
     var = _mm256_insertf128_pd(var, *(const __m128d *)(x), 0);
     var = _mm256_insertf128_pd(var, *(const __m128d *)(x+ivs), 1);
     return var;
}


#define VFMA(a, b, c) VADD(c, VMUL(a, b))
#define VFNMS(a, b, c) VSUB(c, VMUL(a, b))
#define VFMS(a, b, c) VSUB(VMUL(a, b), c)

#define SHUFVAL(fp0,fp1,fp2,fp3) \
   (((fp3) << 3) | ((fp2) << 2) | ((fp1) << 1) | ((fp0)))

static inline V LDA(const R *x, INT ivs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     (void)ivs; /* UNUSED */
     return _mm256_loadu_pd(x);
}

static inline void ST(R *x, V v, INT ovs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     /* WARNING: the extra_iter hack depends upon the store of the low
	part occurring after the store of the high part */
     *(__m128d *)(x + ovs) = _mm256_extractf128_pd(v, 1);
     *(__m128d *)(x) = _mm256_extractf128_pd(v, 0);
}

static inline void STA(R *x, V v, INT ovs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     (void)ovs; /* UNUSED */
     _mm256_storeu_pd(x, v);
}

#define STM2 ST
#define STN2(x, v0, v1, ovs) /* nop */
#define STM4(x, v, ovs, aligned_like) /* no-op */

static inline void STN4(R *x, V v0, V v1, V v2, V v3, INT ovs)
{
     V x0, x1, x2, x3;
     x0 = _mm256_unpacklo_pd(v0, v1);
     x1 = _mm256_unpackhi_pd(v0, v1);
     x2 = _mm256_unpacklo_pd(v2, v3);
     x3 = _mm256_unpackhi_pd(v2, v3);
     STA(x,           _mm256_permute2f128_pd(x0, x2, 0x20), 0, 0);
     STA(x +     ovs, _mm256_permute2f128_pd(x1, x3, 0x20), 0, 0);
     STA(x + 2 * ovs, _mm256_permute2f128_pd(x0, x2, 0x31), 0, 0);
     STA(x + 3 * ovs, _mm256_permute2f128_pd(x1, x3, 0x31), 0, 0);
}

static inline V FLIP_RI(V x)
{
     return VSHUFPD(x, x, SHUFVAL(1, 0, 1, 0));
}

static inline V VCONJ(V x)
{
     V pmpm = VLIT(-0.0, 0.0, -0.0, 0.0);
     return VXOR(pmpm, x);
}

static inline V VBYI(V x)
{
     return FLIP_RI(VCONJ(x));
}

static inline V VZMUL(V tx, V sr)
{
     V tr = VDUPL(tx);
     V ti = VDUPH(tx);
     tr = VMUL(tr, sr);
     sr = VBYI(sr);
     return VADD(tr, VMUL(ti, sr));
}

static inline V VZMULJ(V tx, V sr)
{
     V tr = VDUPL(tx);
     V ti = VDUPH(tx);
     tr = VMUL(tr, sr);
     sr = VBYI(sr);
     return VSUB(tr, VMUL(ti, sr));
}

static inline V VZMULI(V tx, V sr)
{
     V tr = VDUPL(tx);
     V ti = VDUPH(tx);
     ti = VMUL(ti, sr);
     sr = VBYI(sr);
     return VSUB(VMUL(tr, sr), ti);
}

static inline V VZMULIJ(V tx, V sr)
{
     V tr = VDUPL(tx);
     V ti = VDUPH(tx);
     ti = VMUL(ti, sr);
     sr = VBYI(sr);
     return VADD(VMUL(tr, sr), ti);
}

#define VFMAI(b, c) VADD(c, VBYI(b))
#define VFNMSI(b, c) VSUB(c, VBYI(b))

/* twiddle storage #1: compact, slower */
#define VTW1(v,x)  \
  {TW_COS, v, x}, {TW_SIN, v, x}, {TW_COS, v+1, x}, {TW_SIN, v+1, x}
#define TWVL1 (VL)

static inline V BYTW1(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V tx = twp[0];
     V tr = VDUPL(tx);
     V ti = VDUPH(tx);
     tr = VMUL(tr, sr);
     sr = VBYI(sr);
     return VADD(tr, VMUL(ti, sr));
}

static inline V BYTWJ1(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V tx = twp[0];
     V tr = VDUPL(tx);
     V ti = VDUPH(tx);
     tr = VMUL(tr, sr);
     sr = VBYI(sr);
     return VSUB(tr, VMUL(ti, sr));
}

/* twiddle storage #2: twice the space, faster (when in cache) */
#define VTW2(v,x)							\
  {TW_COS, v, x}, {TW_COS, v, x}, {TW_COS, v+1, x}, {TW_COS, v+1, x},	\
  {TW_SIN, v, -x}, {TW_SIN, v, x}, {TW_SIN, v+1, -x}, {TW_SIN, v+1, x}
#define TWVL2 (2 * VL)

static inline V BYTW2(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V si = FLIP_RI(sr);
     V tr = twp[0], ti = twp[1];
     return VADD(VMUL(tr, sr), VMUL(ti, si));
}

static inline V BYTWJ2(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V si = FLIP_RI(sr);
     V tr = twp[0], ti = twp[1];
     return VSUB(VMUL(tr, sr), VMUL(ti, si));
}

/* twiddle storage #3 */
#define VTW3(v,x) {TW_CEXP, v, x}, {TW_CEXP, v+1, x}
#define TWVL3 (VL)

/* twiddle storage for split arrays */
#define VTWS(v,x)							\
  {TW_COS, v, x}, {TW_COS, v+1, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x},	\
  {TW_SIN, v, x}, {TW_SIN, v+1, x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, x}	
#define TWVLS (2 * VL)


#define VFMACONJ(b,c)  VADD(VCONJ(b),c)
#define VFMSCONJ(b,c)  VSUB(VCONJ(b),c)
#define VFNMSCONJ(b,c) VSUB(c, VCONJ(b))

#include "simd-common.h"
