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

#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
typedef R V __attribute__ ((mode(V4SF),aligned(16)));

/* gcc-3.1 seems to generate slower code when we use SSE builtins.
   Use asm instead. */
static __inline__ V VADD(V a, V b)
{
     V ret;
     __asm__("addps %2, %0" : "=x"(ret) : "%0"(a), "xm"(b));
     return ret;
}
static __inline__ V VSUB(V a, V b)
{
     V ret;
     __asm__("subps %2, %0" : "=x"(ret) : "0"(a), "xm"(b));
     return ret;
}
static __inline__ V VMUL(V b, V a)
{
     V ret;
     __asm__("mulps %2, %0" : "=x"(ret) : "%0"(a), "xm"(b));
     return ret;
}
static __inline__ V VXOR(V b, V a)
{
     V ret;
     __asm__("xorps %2, %0" : "=x"(ret) : "%0"(a), "xm"(b));
     return ret;
}
static __inline__ V UNPCKL(V a, V b)
{
     V ret;
     __asm__("unpcklps %2, %0" : "=x"(ret) : "0"(a), "xm"(b));
     return ret;
}

static __inline__ V UNPCKH(V a, V b)
{
     V ret;
     __asm__("unpckhps %2, %0" : "=x"(ret) : "0"(a), "xm"(b));
     return ret;
}

#define SHUFPS(a, b, msk) __extension__ ({				   \
     V ret;								   \
     __asm__("shufps %3, %2, %0" : "=x"(ret) : "0"(a), "xm"(b), "i"(msk)); \
     ret;								   \
})

static __inline__ V LOADH(const R *addr, V val)
{
     V ret;
     __asm__("movhps %2, %0" : "=x"(ret) : "0"(val), "m"(*addr));
     return ret;
}

static __inline__ V LOADL0(const R *addr, V val)
{
     __asm__("movlps %1, %0" : "=x"(val) : "m"(*addr));
     return val;
}


static __inline__ void STOREH(R *addr, V val)
{
     __asm__("movhps %1, %0" : "=m"(*addr) : "x"(val));
}
static __inline__ void STOREL(R *addr, V val)
{
     __asm__("movlps %1, %0" : "=m"(*addr) : "x"(val));
}

#define DVK(var, val) const V var = __extension__ ({		\
     static const union fvec _var = { {val, val, val, val} };	\
     _var.v;							\
})
#define LDK(x) x

#endif

#if defined(__ICC) || defined(_MSC_VER) /* Intel's compiler for ia32 */
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
#endif

#ifdef _MSC_VER
#  define __inline__ __inline
#endif

/* common stuff */
union fvec {
     R f[4];
     V v;
};

#define VFMA(a, b, c) VADD(c, VMUL(a, b))
#define VFNMS(a, b, c) VSUB(c, VMUL(a, b))
#define VFMS(a, b, c) VSUB(VMUL(a, b), c)

#define SHUFVAL(fp0,fp1,fp2,fp3) \
   (((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | ((fp0)))


static __inline__ V LD(const R *x, int ivs, const R *aligned_like)
{
     V var;
     (void)aligned_like; /* UNUSED */
     var = LOADL0(x, var);
     var = LOADH(x + ivs, var);
     return var;
}

static __inline__ V LDA(const R *x, int ivs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     (void)ivs; /* UNUSED */
     return *(const V *)x;
}

static __inline__ void ST(R *x, V v, int ovs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     STOREL(x, v);
     STOREH(x + ovs, v);
}

static __inline__ void STA(R *x, V v, int ovs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     (void)ovs; /* UNUSED */
     *(V *)x = v;
}

static __inline__ V FLIP_RI(V x)
{
     return SHUFPS(x, x, SHUFVAL(1, 0, 3, 2));
}

extern const union fvec X(sse_mpmp);
static __inline__ V CHS_R(V x)
{
     return VXOR(X(sse_mpmp).v, x);
}

static __inline__ V VBYI(V x)
{
     return CHS_R(FLIP_RI(x));
}

#define VFMAI(b, c) VADD(c, VBYI(b))
#define VFNMSI(b, c) VSUB(c, VBYI(b))

#undef FAST_AND_BLOATED_TWIDDLES

#ifdef FAST_AND_BLOATED_TWIDDLES

/* avoid twiddle shuffling by storing twiddles twice in the
   proper format */

#define VTW(x) 								\
  {TW_COS, 0, x}, {TW_COS, 0, x}, {TW_COS, 1, x}, {TW_COS, 1, x},	\
  {TW_SIN, 0, -x}, {TW_SIN, 0, x}, {TW_SIN, 1, -x}, {TW_SIN, 1, x}
#define TWVL (2 * VL)

static __inline__ V BYTW(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V si = FLIP_RI(sr);
     V tr = twp[0], ti = twp[1];
     return VADD(VMUL(tr, sr), VMUL(ti, si));
}

static __inline__ V BYTWJ(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V si = FLIP_RI(sr);
     V tr = twp[0], ti = twp[1];
     return VSUB(VMUL(tr, sr), VMUL(ti, si));
}

#else /* FAST_AND_BLOATED_TWIDDLES */

#define VTW(x) 								\
  {TW_COS, 0, x}, {TW_COS, 1, x}, {TW_SIN, 0, x}, {TW_SIN, 1, x}
#define TWVL (VL)

static __inline__ V BYTW(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V tx = twp[0];
     V tr = UNPCKL(tx, tx);
     V ti = UNPCKH(tx, tx);
     tr = VMUL(tr, sr);
     sr = VBYI(sr);
     return VADD(tr, VMUL(ti, sr));
}

static __inline__ V BYTWJ(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V tx = twp[0];
     V tr = UNPCKL(tx, tx);
     V ti = UNPCKH(tx, tx);
     tr = VMUL(tr, sr);
     sr = VBYI(sr);
     return VSUB(tr, VMUL(ti, sr));
}


#endif /* FAST_AND_BLOATED_TWIDDLES */

#define RIGHT_CPU X(have_sse)
extern int RIGHT_CPU(void);

#define SIMD_VSTRIDE_OKA(x) ((x) == 2)
#define BEGIN_SIMD()
#define END_SIMD()

