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

#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))

#define MY_GNUC_PREREQ(maj, min) \
   ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))

#if MY_GNUC_PREREQ(3, 3)
  /* FIXME: gcc-3.3 supports Intel intrinsics, but the
     current gcc-3.3 snapshot crashes when using them. */
  typedef double V __attribute__ ((mode(V2DF),aligned(16)));
#else
  /* horrible hack because gcc-3.2 and earlier does not support sse2 */
  typedef float V __attribute__ ((mode(V4SF),aligned(16)));
#endif

static __inline__ V VADD(V a, V b)
{
     V ret;
     __asm__("addpd %2, %0" : "=x"(ret) : "%0"(a), "xm"(b));
     return ret;
}
static __inline__ V VSUB(V a, V b)
{
     V ret;
     __asm__("subpd %2, %0" : "=x"(ret) : "0"(a), "xm"(b));
     return ret;
}
static __inline__ V VMUL(V b, V a)
{
     V ret;
     __asm__("mulpd %2, %0" : "=x"(ret) : "%0"(a), "xm"(b));
     return ret;
}

static __inline__ V VXOR(V b, V a)
{
     V ret;
     __asm__("xorpd %2, %0" : "=x"(ret) : "%0"(a), "xm"(b));
     return ret;
}

#define DVK(var, val) V var = __extension__ ({		\
     static const union dvec _var = { {val, val} };	\
     _var.v;						\
})

#define SHUFPD(a, b, msk) __extension__ ({				   \
     V ret;								   \
     __asm__("shufpd %3, %2, %0" : "=x"(ret) : "0"(a), "xm"(b), "i"(msk)); \
     ret;								   \
})

static __inline__ V UNPCKL(V a, V b)
{
     V ret;
     __asm__("unpcklpd %2, %0" : "=x"(ret) : "0"(a), "xm"(b));
     return ret;
}

static __inline__ V UNPCKH(V a, V b)
{
     V ret;
     __asm__("unpckhpd %2, %0" : "=x"(ret) : "0"(a), "xm"(b));
     return ret;
}

#define LDK(x) x
#endif

#if defined(__ICC) || defined(_MSC_VER) /* Intel's compiler for ia32 */
#include <emmintrin.h>

typedef __m128d V;
#define VADD _mm_add_pd
#define VSUB _mm_sub_pd
#define VMUL _mm_mul_pd
#define VXOR _mm_xor_pd
#define DVK(var, val) const R var = K(val)
#define LDK(x) _mm_set1_pd(x)
#define SHUFPD _mm_shuffle_pd
#define UNPCKL _mm_unpacklo_pd
#define UNPCKH _mm_unpackhi_pd
#endif

#ifdef _MSC_VER
#  define __inline__ __inline
#endif

union dvec {
     double d[2];
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

static __inline__ V FLIP_RI(V x)
{
     return SHUFPD(x, x, 1);
}

extern const union dvec X(sse2_mp);
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


static __inline__ V BYTW(const R *t, V sr)
{
     V tx = LD(t, 1, t);
     V tr = UNPCKL(tx, tx);
     V ti = UNPCKH(tx, tx);
     tr = VMUL(sr, tr);
     sr = VBYI(sr);
     return VADD(tr, VMUL(ti, sr));
}

static __inline__ V BYTWJ(const R *t, V sr)
{
     V tx = LD(t, 1, t);
     V tr = UNPCKL(tx, tx);
     V ti = UNPCKH(tx, tx);
     tr = VMUL(sr, tr);
     sr = VBYI(sr);
     return VSUB(tr, VMUL(ti, sr));
}

#define VFMA(a, b, c) VADD(c, VMUL(a, b))
#define VFNMS(a, b, c) VSUB(c, VMUL(a, b))
#define VFMS(a, b, c) VSUB(VMUL(a, b), c)
#define VTW(x) {TW_COS, 0, x}, {TW_SIN, 0, x}
#define TWVL 1

#define RIGHT_CPU X(have_sse2)
extern int RIGHT_CPU(void);

#define SIMD_VSTRIDE_OKA(x) 1
#define BEGIN_SIMD()
#define END_SIMD()

