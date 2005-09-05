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

#ifndef FFTW_SINGLE
#error "SSE only works in single precision"
#endif

#define VL 2            /* SIMD complex vector length */
#define ALIGNMENT 8     /* alignment for LD/ST */
#define ALIGNMENTA 16   /* alignment for LDA/STA */
#define SIMD_VSTRIDE_OKA(x) ((x) == 2)
#define SIMD_STRIDE_OKPAIR SIMD_STRIDE_OK

#define RIGHT_CPU X(have_sse)
extern int RIGHT_CPU(void);

/* gcc compiles the following code only when __SSE__ is defined */
#if defined(__SSE__) || !defined(__GNUC__)

/* some versions of glibc's sys/cdefs.h define __inline to be empty,
   which is wrong because xmmintrin.h defines several inline
   procedures */
#undef __inline

#include <xmmintrin.h>

typedef __m128 V;
#define VADD _mm_add_ps
#define VSUB _mm_sub_ps
#define VMUL _mm_mul_ps
#define VXOR _mm_xor_ps
#define SHUFPS _mm_shuffle_ps
#define LOADH(addr, val) _mm_loadh_pi(val, (const __m64 *)(addr))
#define LOADL0(addr, val) _mm_loadl_pi(val, (const __m64 *)(addr))
#define STOREH(addr, val) _mm_storeh_pi((__m64 *)(addr), val)
#define STOREL(addr, val) _mm_storel_pi((__m64 *)(addr), val)
#define UNPCKH _mm_unpackhi_ps
#define UNPCKL _mm_unpacklo_ps
#define DVK(var, val) const R var = K(val)
#define LDK(x) _mm_set_ps1(x)

union fvec {
     R f[4];
     V v;
};

union uvec {
     unsigned u[4];
     V v;
};

#define VFMA(a, b, c) VADD(c, VMUL(a, b))
#define VFNMS(a, b, c) VSUB(c, VMUL(a, b))
#define VFMS(a, b, c) VSUB(VMUL(a, b), c)

#define SHUFVAL(fp0,fp1,fp2,fp3) \
   (((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | ((fp0)))


static inline V LD(const R *x, int ivs, const R *aligned_like)
{
     V var;
     (void)aligned_like; /* UNUSED */
     var = LOADL0(x, var);
     var = LOADH(x + ivs, var);
     return var;
}

static inline V LDA(const R *x, int ivs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     (void)ivs; /* UNUSED */
     return *(const V *)x;
}

static inline void ST(R *x, V v, int ovs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     STOREL(x, v);
     STOREH(x + ovs, v);
}

static inline void STA(R *x, V v, int ovs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     (void)ovs; /* UNUSED */
     *(V *)x = v;
}

#if 0
/* this should be faster but it isn't. */
static inline void STPAIR2(R *x, V v0, V v1, int ovs)
{
     STA(x, SHUFPS(v0, v1, SHUFVAL(0, 1, 0, 1)), ovs, 0);
     STA(x + ovs, SHUFPS(v0, v1, SHUFVAL(2, 3, 2, 3)), ovs, 0);
}
#endif
#define STPAIR1 ST
#define STPAIR2(x, v0, v1, ovs) /* nop */

static inline V FLIP_RI(V x)
{
     return SHUFPS(x, x, SHUFVAL(1, 0, 3, 2));
}

extern const union uvec X(sse_mpmp);
static inline V CHS_R(V x)
{
     return VXOR(X(sse_mpmp).v, x);
}

static inline V VBYI(V x)
{
     return CHS_R(FLIP_RI(x));
}

#define VFMAI(b, c) VADD(c, VBYI(b))
#define VFNMSI(b, c) VSUB(c, VBYI(b))

/* twiddle storage #1: compact, slower */
#define VTW1(x) {TW_COS, 0, x}, {TW_COS, 1, x}, {TW_SIN, 0, x}, {TW_SIN, 1, x}
#define TWVL1 (VL)

static inline V BYTW1(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V tx = twp[0];
     V tr = UNPCKL(tx, tx);
     V ti = UNPCKH(tx, tx);
     tr = VMUL(tr, sr);
     sr = VBYI(sr);
     return VADD(tr, VMUL(ti, sr));
}

static inline V BYTWJ1(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V tx = twp[0];
     V tr = UNPCKL(tx, tx);
     V ti = UNPCKH(tx, tx);
     tr = VMUL(tr, sr);
     sr = VBYI(sr);
     return VSUB(tr, VMUL(ti, sr));
}

/* twiddle storage #2: twice the space, faster (when in cache) */
#define VTW2(x)								\
  {TW_COS, 0, x}, {TW_COS, 0, x}, {TW_COS, 1, x}, {TW_COS, 1, x},	\
  {TW_SIN, 0, -x}, {TW_SIN, 0, x}, {TW_SIN, 1, -x}, {TW_SIN, 1, x}
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

#define BEGIN_SIMD()
#define END_SIMD()

#endif /* __SSE__ */
