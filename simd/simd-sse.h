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
typedef float V __attribute__ ((mode(V4SF),aligned(16)));

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

#define SHUFPS(a, b, msk) __extension__ ({				   \
     V ret;								   \
     __asm__("shufps %3, %2, %0" : "=x"(ret) : "0"(a), "xm"(b), "i"(msk)); \
     ret;								   \
})

union fvec { 
     float f[4];
     V v;
};

#define DVK(var, val) const V var = __extension__ ({		\
     static const union fvec _var = { {val, val, val, val} };	\
     _var.v;							\
})

#define LD(var, loc) var = *(const V *)(&(loc))
#define ST(loc, var) *(V *)(&(loc)) = var
#define LDK(x) x

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

#endif

#ifdef __ICC /* Intel's compiler for ia32 */
#include <xmmintrin.h>

typedef __m128 V;
#define VADD _mm_add_ps
#define VSUB _mm_sub_ps
#define VMUL _mm_mul_ps
#define LD(var, loc) var = *(const V *)(&(loc))
#define ST(loc, var) *(V *)(&(loc)) = var
#define DVK(var, val) const R var = K(val)
#define LDK(x) _mm_set_ps1(x)

#define ST4(a, ovs, v0, v1, v2, v3)		\
{						\
     R *_b = &(a);				\
						\
     _MM_TRANSPOSE4_PS(v0, v1, v2, v3);		\
						\
     ST(_b[0 * ovs], v0);			\
     ST(_b[1 * ovs], v1);			\
     ST(_b[2 * ovs], v2);			\
     ST(_b[3 * ovs], v3);			\
}
#endif

#define VFMA(a, b, c) VADD(c, VMUL(a, b))
#define VFNMS(a, b, c) VSUB(c, VMUL(a, b))

#define VTW(op, x) {op, 0, x}, {op, 1, x}, {op, 2, x}, {op, 3, x}

#define RIGHT_CPU X(have_sse)
extern int RIGHT_CPU(void);
