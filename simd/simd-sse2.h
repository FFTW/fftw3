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

static __inline__ V LDA(const R *x, int ivs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     (void)ivs; /* UNUSED */
     return *(const V *)x;
}

static __inline__ void STA(R *x, V v, int ovs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     (void)ovs; /* UNUSED */
     *(V *)x = v;
}

#define LD LDA
#define ST STA

#define STM2 STA
#define STN2(x, v0, v1, ovs) /* nop */

#define SSE2_USE_STM4
static inline void STM4(R *x, V v, int ovs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
#ifdef SSE2_USE_STM4
     STOREL(x, v);
     STOREH(x + ovs, v);
#endif
}

static inline void STN4(R *x, V v0, V v1, V v2, V v3, int ovs)
{
     (void)x;   /* UNUSED */
     (void)v0;  /* UNUSED */
     (void)v1;  /* UNUSED */
     (void)v2;  /* UNUSED */
     (void)v3;  /* UNUSED */
     (void)ovs; /* UNUSED */
#ifndef SSE2_USE_STM4
     STA(x, UNPCKL(v0, v1), 0, 0);
     STA(x + ovs, UNPCKH(v0, v1), 0, 0);
     STA(x + 2, UNPCKL(v2, v3), 0, 0);
     STA(x + 2 + ovs, UNPCKH(v2, v3), 0, 0);
#endif
}

static __inline__ V FLIP_RI(V x)
{
     return SHUFPD(x, x, 1);
}

extern const union uvec X(sse2_mp);
static __inline__ V CHS_R(V x)
{
     return VXOR(X(sse2_mp).v, x);
}

static __inline__ V VBYI(V x)
{
     x = FLIP_RI(x);
     x = CHS_R(x);
     return x;
}

#define VFMAI(b, c) VADD(c, VBYI(b))
#define VFNMSI(b, c) VSUB(c, VBYI(b))

/* twiddle storage #1: compact, slower */
#define VTW1(x) {TW_COS, 0, x}, {TW_SIN, 0, x}
#define TWVL1 1

static __inline__ V BYTW1(const R *t, V sr)
{
     V tx = LD(t, 1, t);
     V tr = UNPCKL(tx, tx);
     V ti = UNPCKH(tx, tx);
     tr = VMUL(sr, tr);
     sr = VBYI(sr);
     return VADD(tr, VMUL(ti, sr));
}

static __inline__ V BYTWJ1(const R *t, V sr)
{
     V tx = LD(t, 1, t);
     V tr = UNPCKL(tx, tx);
     V ti = UNPCKH(tx, tx);
     tr = VMUL(sr, tr);
     sr = VBYI(sr);
     return VSUB(tr, VMUL(ti, sr));
}

/* twiddle storage #2: twice the space, faster (when in cache) */
#define VTW2(x)								\
  {TW_COS, 0, x}, {TW_COS, 0, x}, {TW_SIN, 0, -x}, {TW_SIN, 0, x}
#define TWVL2 2

static __inline__ V BYTW2(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V si = FLIP_RI(sr);
     V tr = twp[0], ti = twp[1];
     return VADD(VMUL(tr, sr), VMUL(ti, si));
}

static __inline__ V BYTWJ2(const R *t, V sr)
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
#define VTWS(x)								\
  {TW_COS, 0, x}, {TW_COS, 1, x}, {TW_SIN, 0, x}, {TW_SIN, 1, x}
#define TWVLS (2 * VL)

#define BEGIN_SIMD()
#define END_SIMD()

#endif /* __SSE2__ */
