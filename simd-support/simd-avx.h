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

#if defined(FFTW_LDOUBLE) || defined(FFTW_QUAD)
#error "AVX only works in single or double precision"
#endif

#ifdef FFTW_SINGLE
#  define DS(d,s) s /* single-precision option */
#  define SUFF(name) name ## s
#else
#  define DS(d,s) d /* double-precision option */
#  define SUFF(name) name ## d
#endif

#define SIMD_SUFFIX  _avx  /* for renaming */
#define VL DS(2, 4)        /* SIMD complex vector length */
#define SIMD_VSTRIDE_OKA(x) ((x) == 2) 
#define SIMD_STRIDE_OKPAIR SIMD_STRIDE_OK

#if defined(__GNUC__) && !defined(__AVX__) /* sanity check */
#error "compiling simd-avx.h without -mavx"
#endif

#include <immintrin.h>

typedef DS(__m256d, __m256) V;
#define VADD SUFF(_mm256_add_p)
#define VSUB SUFF(_mm256_sub_p)
#define VMUL SUFF(_mm256_mul_p)
#define VXOR SUFF(_mm256_xor_p)
#define VSHUF SUFF(_mm256_shuffle_p)

#define SHUFVALD(fp0,fp1) \
   (((fp1) << 3) | ((fp0) << 2) | ((fp1) << 1) | ((fp0)))
#define SHUFVALS(fp0,fp1,fp2,fp3) \
   (((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | ((fp0)))

#define VDUPL(x) DS(_mm256_unpacklo_pd(x, x), VSHUF(x, x, SHUFVALS(0, 0, 2, 2)))
#define VDUPH(x) DS(_mm256_unpackhi_pd(x, x), VSHUF(x, x, SHUFVALS(1, 1, 3, 3)))

#define VLIT(x0, x1) DS(_mm256_set_pd(x0, x1, x0, x1), _mm256_set_ps(x0, x1, x0, x1, x0, x1, x0, x1))
#define DVK(var, val) V var = VLIT(val, val)
#define LDK(x) x

static inline V LDA(const R *x, INT ivs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     (void)ivs; /* UNUSED */
     return SUFF(_mm256_loadu_p)(x);
}

static inline void STA(R *x, V v, INT ovs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     (void)ovs; /* UNUSED */
     SUFF(_mm256_storeu_p)(x, v);
}

#if FFTW_SINGLE
union rvec {
     R r[8];
     V v;
};

static inline V LD(const R *x, INT ivs, const R *aligned_like)
{
     /* FIXME: slow */
     union rvec r;
     (void)aligned_like; /* UNUSED */
     r.r[0] = x[0];
     r.r[1] = x[1];
     r.r[2] = x[ivs];
     r.r[3] = x[ivs+1];
     r.r[4] = x[2*ivs];
     r.r[5] = x[2*ivs+1];
     r.r[6] = x[3*ivs];
     r.r[7] = x[3*ivs+1];
     return r.v;
}

static inline void ST(R *x, V v, INT ovs, const R *aligned_like)
{
     union rvec r;
     (void)aligned_like; /* UNUSED */
     r.v = v;
     x[0] = r.r[0];
     x[1] = r.r[1];
     x[ovs] = r.r[2];
     x[ovs+1] = r.r[3];
     x[2*ovs] = r.r[4];
     x[2*ovs+1] = r.r[5];
     x[3*ovs] = r.r[6];
     x[3*ovs+1] = r.r[7];
}

#define STM2 ST
#define STN2(x, v0, v1, ovs) /* nop */

static inline void STM4(R *x, V v, INT ovs, const R *aligned_like)
{
     union rvec r;
     (void)aligned_like; /* UNUSED */
     r.v = v;
     x[0*ovs] = r.r[0];
     x[1*ovs] = r.r[1];
     x[2*ovs] = r.r[2];
     x[3*ovs] = r.r[3];
     x[4*ovs] = r.r[4];
     x[5*ovs] = r.r[5];
     x[6*ovs] = r.r[6];
     x[7*ovs] = r.r[7];
}
#define STN4(x, v0, v1, v2, v3, ovs)  /* nop */

#else
static inline __m128d LOAD128(const R *x)
{
     /* gcc-4.6 miscompiles the combination _mm256_castpd128_pd256(LOAD128(x))
	into a 256-bit vmovapd, which requires 32-byte aligment instead of
	16-byte alignment.

	icc-12.0.0 generates movaps, which is an SSE instruction and subject
	to the AVX<->SSE transition penalty.  (What were they thinking?).

	Force the use of vmovapd via asm until compilers stabilize.
     */
#if defined(__GNUC__) || (defined(__ICC) && defined(unix))
     __m128d var;
     __asm__("vmovapd %1, %0\n" : "=x"(var) : "m"(x[0]));
     return var;
#else
     return *(const __m128d *)x;
#endif
}

static inline void STORE128(R *x, __m128d var)
{
     /* icc-12.0.0 generates movaps, which is an SSE instruction and subject
	to the AVX<->SSE transition penalty.

	Force the use of vmovapd via asm.  Note that this instruction
	ought to be combined with extractf128, but it isn't in the
	current formulation. */
#if (defined(__ICC) && defined(unix))
     __asm__("vmovapd %0, %1\n" :: "x"(var), "m"(x[0]));
#else
     *(__m128d *)x = var;
#endif
}

static inline V LD(const R *x, INT ivs, const R *aligned_like)
{
     V var;
     (void)aligned_like; /* UNUSED */
     var = _mm256_castpd128_pd256(LOAD128(x));
     var = _mm256_insertf128_pd(var, *(const __m128d *)(x+ivs), 1);
     return var;
}

static inline void ST(R *x, V v, INT ovs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     /* WARNING: the extra_iter hack depends upon the store of the low
	part occurring after the store of the high part */
     *(__m128d *)(x + ovs) = _mm256_extractf128_pd(v, 1);
     STORE128(x, _mm256_extractf128_pd(v, 0));
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
#endif

static inline V FLIP_RI(V x)
{
     return VSHUF(x, x,
		  DS(SHUFVALD(1, 0), 
		     SHUFVALS(1, 0, 3, 2)));
}

static inline V VCONJ(V x)
{
     V pmpm = VLIT(-0.0, 0.0);
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

/* twiddle storage #1: compact, slower */
#ifdef FFTW_SINGLE
# define VTW1(v,x) {TW_CEXP, v, x}, {TW_CEXP, v+1, x}, {TW_CEXP, v+2, x}, {TW_CEXP, v+3, x}
#else
# define VTW1(v,x) {TW_CEXP, v, x}, {TW_CEXP, v+1, x}
#endif
#define TWVL1 (VL)

static inline V BYTW1(const R *t, V sr)
{
     return VZMUL(LDA(t, 2, t), sr);
}

static inline V BYTWJ1(const R *t, V sr)
{
     return VZMULJ(LDA(t, 2, t), sr);
}

/* twiddle storage #2: twice the space, faster (when in cache) */
#ifdef FFTW_SINGLE
# define VTW2(v,x)							\
   {TW_COS, v, x}, {TW_COS, v, x}, {TW_COS, v+1, x}, {TW_COS, v+1, x},	\
   {TW_COS, v+2, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, {TW_COS, v+3, x}, \
   {TW_SIN, v, -x}, {TW_SIN, v, x}, {TW_SIN, v+1, -x}, {TW_SIN, v+1, x}, \
   {TW_SIN, v+2, -x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, -x}, {TW_SIN, v+3, x}
#else
# define VTW2(v,x)							\
   {TW_COS, v, x}, {TW_COS, v, x}, {TW_COS, v+1, x}, {TW_COS, v+1, x},	\
   {TW_SIN, v, -x}, {TW_SIN, v, x}, {TW_SIN, v+1, -x}, {TW_SIN, v+1, x}
#endif
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
#define VTW3 VTW1
#define TWVL3 TWVL1

/* twiddle storage for split arrays */
#ifdef FFTW_SINGLE
# define VTWS(v,x)							\
  {TW_COS, v, x}, {TW_COS, v+1, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x},	\
  {TW_COS, v+4, x}, {TW_COS, v+5, x}, {TW_COS, v+6, x}, {TW_COS, v+7, x}, \
  {TW_SIN, v, x}, {TW_SIN, v+1, x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, x},	\
  {TW_SIN, v+4, x}, {TW_SIN, v+5, x}, {TW_SIN, v+6, x}, {TW_SIN, v+7, x}
#else
# define VTWS(v,x)							\
  {TW_COS, v, x}, {TW_COS, v+1, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x},	\
  {TW_SIN, v, x}, {TW_SIN, v+1, x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, x}	
#endif
#define TWVLS (2 * VL)


/* FMA macros */
#define VFMA(a, b, c) VADD(c, VMUL(a, b))
#define VFNMS(a, b, c) VSUB(c, VMUL(a, b))
#define VFMS(a, b, c) VSUB(VMUL(a, b), c)
#define VFMAI(b, c) VADD(c, VBYI(b))
#define VFNMSI(b, c) VSUB(c, VBYI(b))
#define VFMACONJ(b,c)  VADD(VCONJ(b),c)
#define VFMSCONJ(b,c)  VSUB(VCONJ(b),c)
#define VFNMSCONJ(b,c) VSUB(c, VCONJ(b))

/* User VZEROUPPER to avoid the penalty of switching from AVX to
   SSE.   See Intel Optimization Manual (April 2011, version 248966),
   Section 11.3 */
#define VLEAVE _mm256_zeroupper

#include "simd-common.h"
