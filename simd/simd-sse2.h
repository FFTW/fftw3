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

#if defined(FFTW_SINGLE) || defined(FFTW_LDOUBLE)
#error "SSE2 only works in double precision"
#endif

#define VL 1            /* SIMD vector length, in term of complex numbers */
#define ALIGNMENT 16
#define ALIGNMENTA 16
#define SIMD_VSTRIDE_OKA(x) 1
#define SIMD_STRIDE_OKPAIR SIMD_STRIDE_OK

#define RIGHT_CPU X(have_sse2)
extern int RIGHT_CPU(void);

/* gcc compiles the following code only when __SSE2__ is defined */
#if defined(__SSE2__) || !defined(__GNUC__)

/* some versions of glibc's sys/cdefs.h define __inline to be empty,
   which is wrong because emmintrin.h defines several inline
   procedures */
#undef __inline

#include <emmintrin.h>

typedef __m128d V;
#define VADD _mm_add_pd
#define VSUB _mm_sub_pd
#define VMUL _mm_mul_pd
#define VXOR _mm_xor_pd
#define SHUFPD _mm_shuffle_pd
#define UNPCKL _mm_unpacklo_pd
#define UNPCKH _mm_unpackhi_pd
#define STOREH _mm_storeh_pd
#define STOREL _mm_storel_pd

#ifdef __GNUC__
#define DVK(var, val) V var = __extension__ ({		\
     static const union dvec _var = { {val, val} };	\
     _var.v;						\
})
#define LDK(x) x
#else
#define DVK(var, val) const R var = K(val)
#define LDK(x) _mm_set1_pd(x)
#endif

union dvec {
     double d[2];
     V v;
};

union uvec {
     unsigned u[4];
     V v;
};

static inline V LDA(const R *x, INT ivs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     (void)ivs; /* UNUSED */
     return *(const V *)x;
}

static inline void STA(R *x, V v, INT ovs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     (void)ovs; /* UNUSED */
     *(V *)x = v;
}

#define LD LDA
#define ST STA

#define STM2 STA
#define STN2(x, v0, v1, ovs) /* nop */

static inline void STM4(R *x, V v, INT ovs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     STOREL(x, v);
     STOREH(x + ovs, v);
}

#define STN4(x, v0, v1, v2, v3, ovs) /* nothing */

#if 0 /* not used, using STM4 */
static inline void STN4(R *x, V v0, V v1, V v2, V v3, INT ovs)
{
     (void)x;   /* UNUSED */
     (void)v0;  /* UNUSED */
     (void)v1;  /* UNUSED */
     (void)v2;  /* UNUSED */
     (void)v3;  /* UNUSED */
     (void)ovs; /* UNUSED */
     STA(x, UNPCKL(v0, v1), 0, 0);
     STA(x + ovs, UNPCKH(v0, v1), 0, 0);
     STA(x + 2, UNPCKL(v2, v3), 0, 0);
     STA(x + 2 + ovs, UNPCKH(v2, v3), 0, 0);
}
#endif

static inline V FLIP_RI(V x)
{
     return SHUFPD(x, x, 1);
}

extern const union uvec X(sse2_pm);
static inline V VCONJ(V x)
{
     return VXOR(X(sse2_pm).v, x);
}

static inline V VBYI(V x)
{
     x = VCONJ(x);
     x = FLIP_RI(x);
     return x;
}

#define VFMAI(b, c) VADD(c, VBYI(b))
#define VFNMSI(b, c) VSUB(c, VBYI(b))

static inline V VZMUL(V tx, V sr)
{
     V tr = UNPCKL(tx, tx);
     V ti = UNPCKH(tx, tx);
     tr = VMUL(sr, tr);
     sr = VBYI(sr);
     return VADD(tr, VMUL(ti, sr));
}

static inline V VZMULJ(V tx, V sr)
{
     V tr = UNPCKL(tx, tx);
     V ti = UNPCKH(tx, tx);
     tr = VMUL(sr, tr);
     sr = VBYI(sr);
     return VSUB(tr, VMUL(ti, sr));
}

static inline V VZMULI(V tx, V sr)
{
     V tr = UNPCKL(tx, tx);
     V ti = UNPCKH(tx, tx);
     ti = VMUL(ti, sr);
     sr = VBYI(sr);
     return VSUB(VMUL(tr, sr), ti);
}

static inline V VZMULIJ(V tx, V sr)
{
     V tr = UNPCKL(tx, tx);
     V ti = UNPCKH(tx, tx);
     ti = VMUL(ti, sr);
     sr = VBYI(sr);
     return VADD(VMUL(tr, sr), ti);
}

/* twiddle storage #1: compact, slower */
#define VTW1(v,x) {TW_CEXP, v, x}
#define TWVL1 1
#define VTW3(v,x) VTW1(v,x)
#define TWVL3 TWVL1

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
#define VTW2(v,x)							\
  {TW_COS, v, x}, {TW_COS, v, x}, {TW_SIN, v, -x}, {TW_SIN, v, x}
#define TWVL2 2

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

#define VFMA(a, b, c) VADD(c, VMUL(a, b))
#define VFNMS(a, b, c) VSUB(c, VMUL(a, b))
#define VFMS(a, b, c) VSUB(VMUL(a, b), c)

/* twiddle storage for split arrays */
#define VTWS(v,x)							\
  {TW_COS, v, x}, {TW_COS, v+1, x}, {TW_SIN, v, x}, {TW_SIN, v+1, x}
#define TWVLS (2 * VL)

#endif /* __SSE2__ */
