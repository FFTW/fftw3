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

#define VL 2            /* SIMD vector length */
#define ALIGNMENT 16

#if defined(__GNUC__) && defined(__i386__)

/* horrible hack because gcc does not support sse2 yet */
typedef float V __attribute__ ((mode(V4SF)));
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
static __inline__ V VMUL(V a, V b) 
{
     V ret;
     __asm__("mulpd %2, %0" : "=x"(ret) : "0"(a), "xm"(b));
     return ret;
}

static __inline__ V VUNPACKLO(V a, V b) 
{
     V ret;
     __asm__("unpcklpd %2, %0" : "=x"(ret) : "0"(a), "xm"(b));
     return ret;
}

static __inline__ V VUNPACKHI(V a, V b) 
{
     V ret;
     __asm__("unpckhpd %2, %0" : "=x"(ret) : "0"(a), "xm"(b));
     return ret;
}

#define LD(var, loc) var = *(const V *)(&(loc))
#define ST(loc, var) *(V *)(&(loc)) = var
#define DVK(var, val) V var = (						\
{									\
     static const union { double d[2]; V v; } _var = { {val, val} };	\
     _var.v;								\
})

#define ST2(a, ovs, s0, s1)			\
{						\
     R *_b = &(a);				\
     V _v0 = VUNPACKLO(s0, s1);			\
     V _v1 = VUNPACKHI(s0, s1);			\
     ST(_b[0 * ovs], _v0);			\
     ST(_b[1 * ovs], _v1);			\
}

#define ST4(a, ovs, v0, v1, v2, v3)		\
{						\
     ST2(a, ovs, v0, v1);			\
     ST2((&(a))[2], ovs, v2, v3);		\
}

#endif



#ifdef __ICC /* Intel's compiler for ia32 */
#include <emmintrin.h>

typedef __m128d V;
#define VADD _mm_add_pd
#define VSUB _mm_sub_pd
#define VMUL _mm_mul_pd
#define LD(var, loc) var = *(const V *)(&(loc))
#define ST(loc, var) *(V *)(&(loc)) = var
#define DVK(var, val) V var = _mm_set1_pd(val)

#define ST2(a, ovs, s0, s1)			\
{						\
     R *_b = &(a);				\
     V _v0 = _mm_unpacklo_pd(s0, s1);		\
     V _v1 = _mm_unpackhi_pd(s0, s1);		\
     ST(_b[0 * ovs], _v0);			\
     ST(_b[1 * ovs], _v1);			\
}

#define ST4(a, ovs, v0, v1, v2, v3)		\
{						\
     ST2(a, ovs, v0, v1);			\
     ST2((&(a))[2], ovs, v2, v3);		\
}
#endif

#define VFMA(a, b, c) VADD(c, VMUL(a, b))
#define VFNMS(a, b, c) VSUB(c, VMUL(a, b))

#define VTW(op, x) {op, 0, x}, {op, 1, x}

#define RIGHT_CPU X(have_sse2)
extern int RIGHT_CPU(void);
