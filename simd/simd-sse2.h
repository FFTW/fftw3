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


static __inline__ V LOADH(const R *addr, V val)
{
     V ret;
     __asm__("movhpd %2, %0" : "=x"(ret) : "0"(val), "m"(*addr));
     return ret;
}

static __inline__ V LOADL0(const R *addr, V val)
{
     __asm__("movlpd %1, %0" : "=x"(val) : "m"(*addr));
     return val;
}


static __inline__ void STOREH(R *addr, V val)
{
     __asm__("movhpd %1, %0" : "=m"(*addr) : "x"(val));
}
static __inline__ void STOREL(R *addr, V val)
{
     __asm__("movlpd %1, %0" : "=m"(*addr) : "x"(val));
}

union dvec { 
     double d[2];
     V v;
};

#define DVK(var, val) V var = __extension__ ({		\
     static const union dvec _var = { {val, val} };	\
     _var.v;						\
})

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
#define VUNPACKLO _mm_unpacklo_pd
#define VUNPACKHI _mm_unpackhi_pd
#endif

#define LD(var, loc) var = *(const V *)(&(loc))
#define ST(loc, var) *(V *)(&(loc)) = var

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

#define STRI ST2 /* hack */

#define LDRI(r, i, a, ovs)			\
{						\
     const R *_b = &(a);			\
     V _v0, _v1;				\
     LD(_v0, _b[0 * ovs]);			\
     LD(_v1, _b[1 * ovs]);			\
     r = VUNPACKLO(_v0, _v1);			\
     i = VUNPACKHI(_v0, _v1);			\
}

#define STRI2(a, r, i) STRI(a, 2, r, i)
#define LDRI2(r, i, a) LDRI(r, i, a, 2)

#define VFMA(a, b, c) VADD(c, VMUL(a, b))
#define VFNMS(a, b, c) VSUB(c, VMUL(a, b))

#define VTW(op, x) {op, 0, x}, {op, 1, x}

#define RIGHT_CPU X(have_sse2)
extern int RIGHT_CPU(void);
