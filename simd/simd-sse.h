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

#ifndef FFTW_SINGLE
#error "SSE only works in single precision"
#endif

#define VL 4            /* SIMD vector length */
#define ALIGNMENT 16

#if defined(__GNUC__) && defined(__i386__)
typedef R V __attribute__ ((mode(V4SF),aligned(16)));

/* gcc-3.1 seems to generate slower code when we use SSE builtins.
   Use asm instead. */
static __inline__ V VADD(V a, V b) 
{
     V ret;
     __asm__("addps %2, %0" : "=x"(ret) : "0"(a), "xm"(b));
     return ret;
}
static __inline__ V VSUB(V a, V b) 
{
     V ret;
     __asm__("subps %2, %0" : "=x"(ret) : "0"(a), "xm"(b));
     return ret;
}
static __inline__ V VMUL(V a, V b) 
{
     V ret;
     __asm__("mulps %2, %0" : "=x"(ret) : "0"(a), "xm"(b));
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


union fvec { 
     R f[4];
     V v;
};

#define DVK(var, val) const V var = __extension__ ({		\
     static const union fvec _var = { {val, val, val, val} };	\
     _var.v;							\
})
#define LDK(x) x

#endif

#ifdef __ICC /* Intel's compiler for ia32 */
#include <xmmintrin.h>

typedef __m128 V;
#define VADD _mm_add_ps
#define VSUB _mm_sub_ps
#define VMUL _mm_mul_ps
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


/* common stuff */
#define VFMA(a, b, c) VADD(c, VMUL(a, b))
#define VFNMS(a, b, c) VSUB(c, VMUL(a, b))
#define LD(var, loc) var = *(const V *)(&(loc))
#define ST(loc, var) *(V *)(&(loc)) = var
#define VTW(op, x) {op, 0, x}, {op, 1, x}, {op, 2, x}, {op, 3, x}

#define SHUFVAL(fp3,fp2,fp1,fp0) \
   (((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | ((fp0)))

#define VTR4(r0, r1, r2, r3)			\
{						\
     V _t0, _t1, _t2, _t3;			\
						\
						\
     _t0 = SHUFPS((r0), (r1), 0x44);		\
     _t2 = SHUFPS((r0), (r1), 0xEE);		\
     _t1 = SHUFPS((r2), (r3), 0x44);		\
     _t3 = SHUFPS((r2), (r3), 0xEE);		\
						\
     r0 = SHUFPS(_t0, _t1, 0x88);		\
     r1 = SHUFPS(_t0, _t1, 0xDD);		\
     r2 = SHUFPS(_t2, _t3, 0x88);		\
     r3 = SHUFPS(_t2, _t3, 0xDD);		\
}

#define ST4(a, ovs, v0, v1, v2, v3)		\
{						\
     R *_b = &(a);				\
						\
     VTR4(v0, v1, v2, v3);			\
						\
     ST(_b[0 * ovs], v0);			\
     ST(_b[1 * ovs], v1);			\
     ST(_b[2 * ovs], v2);			\
     ST(_b[3 * ovs], v3);			\
}

#define LDRI(r, i, a, ivs)			\
{						\
     V _t0, _t1;				\
     const R *_b = &(a);			\
     _t0 = LOADL0(_b, _t0);			\
     _t0 = LOADH(_b + ivs, _t0);		\
     _t1 = LOADL0(_b + 2 * ivs, _t1);		\
     _t1 = LOADH(_b + 3 * ivs, _t1);		\
     r = SHUFPS(_t0, _t1, SHUFVAL(2, 0, 2, 0));	\
     i = SHUFPS(_t0, _t1, SHUFVAL(3, 1, 3, 1));	\
}						

#define LDRI2(r, i, a)				\
{						\
     V _t0, _t1;				\
     const R *_b = &(a);			\
     LD(_t0, _b[0]);				\
     LD(_t1, _b[4]);				\
     r = SHUFPS(_t0, _t1, SHUFVAL(2, 0, 2, 0));	\
     i = SHUFPS(_t0, _t1, SHUFVAL(3, 1, 3, 1)); \
}

#define STRI(a, ovs, r, i)			\
{						\
     V _t0, _t1;				\
     R *_b = &(a);				\
     _t0 = UNPCKL(r, i);			\
     _t1 = UNPCKH(r, i);			\
     STOREL(_b, _t0);				\
     STOREH(_b + ovs, _t0);			\
     STOREL(_b + 2 * ovs, _t1);			\
     STOREH(_b + 3 * ovs, _t1);			\
}

#define STRI2(a, r, i)				\
{						\
     V _t0, _t1;				\
     R *_b = &(a);				\
     _t0 = UNPCKL(r, i);			\
     _t1 = UNPCKH(r, i);			\
     ST(_b[0], _t0);				\
     ST(_b[4], _t1);				\
}

#define RIGHT_CPU X(have_sse)
extern int RIGHT_CPU(void);
