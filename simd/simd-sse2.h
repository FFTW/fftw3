/*
 * Copyright (c) 2002 Matteo Frigo
 * Copyright (c) 2002 Steven G. Johnson
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

#ifdef FFTW_SINGLE
#error "SSE2 only works in double precision"
#endif

#define VL 1            /* SIMD vector length, in term of complex numbers */
#define ALIGNMENT 16

#if defined(__GNUC__) && defined(__i386__)

/* horrible hack because gcc does not support sse2 yet */
typedef float V __attribute__ ((mode(V4SF),aligned(16)));

static __inline__ V VADD(V a, V b) 
{
     V ret;
     __asm__("addpd %2, %0" : "=x"(ret) : "0"(a), "xm"(b));
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
     __asm__("mulpd %2, %0" : "=x"(ret) : "0"(a), "xm"(b));
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


#ifdef __ICC /* Intel's compiler for ia32 */
#include <emmintrin.h>

typedef __m128d V;
#define VADD _mm_add_pd
#define VSUB _mm_sub_pd
#define VMUL _mm_mul_pd
#define DVK(var, val) const R var = K(val)
#define LDK(x) _mm_set1_pd(x)
#define SHUFPD _mm_shuffle_pd
#define UNPCKL _mm_unpacklo_pd
#define UNPCKH _mm_unpackhi_pd
#endif

union dvec { 
     double d[2];
     V v;
};

#define LD(loc, ivs) (*(const V *)(loc))
#define ST(loc, var, ovs) *(V *)(loc) = var

extern const union dvec X(sse2_p1m1);
extern const union dvec X(sse2_m1p1);

static __inline__ V VBYI(V x)
{
     V y = VMUL(X(sse2_p1m1).v, x);
     return SHUFPD(y, y, 1);
}

#define VFMAI(b, c) VADD(c, VBYI(b))
#define VFNMSI(b, c) VSUB(c, VBYI(b))

static __inline__ V BYTW(const R *t, V c_d)
{
     V a_b = LD(t, 1);
     V d_c = SHUFPD(c_d, c_d, 1);
     V ac_bd = VMUL(a_b, c_d);
     V ad_bc = VMUL(a_b, d_c);
     V ac_ad = UNPCKL(ac_bd, ad_bc);
     V bd_bc = UNPCKH(ac_bd, ad_bc);
     V mbd_bc = VMUL(X(sse2_m1p1).v, bd_bc);
     V x = VADD(mbd_bc, ac_ad);
     return x;
}

static __inline__ V BYTWJ(const R *t, V c_d)
{
     V a_mb = LD(t, 1);
     V d_c = SHUFPD(c_d, c_d, 1);
     V ac_mbd = VMUL(a_mb, c_d);
     V ad_mbc = VMUL(a_mb, d_c);
     V ac_ad = UNPCKL(ac_mbd, ad_mbc);
     V mbd_mbc = UNPCKH(ac_mbd, ad_mbc);
     V mbd_bc = VMUL(X(sse2_p1m1).v, mbd_mbc);
     V x = VADD(mbd_bc, ac_ad);
     return x;
}


#define VFMA(a, b, c) VADD(c, VMUL(a, b))
#define VFNMS(a, b, c) VSUB(c, VMUL(a, b))
#define VFMS(a, b, c) VSUB(VMUL(a, b), c)
#define VTW(x) {TW_COS, 0, x}, {TW_SIN, 0, x}
#define TWVL 1

#define RIGHT_CPU X(have_sse2)
extern int RIGHT_CPU(void);
