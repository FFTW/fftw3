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
typedef float V __attribute__ ((mode(V4SF)));
#define VADD(a, b) __builtin_ia32_addps(a, b)
#define VSUB(a, b) __builtin_ia32_subps(a, b)
#define VMUL(a, b) __builtin_ia32_mulps(b, a)
#define LD(var, loc) var = *(const V *)(&(loc))
#define ST(loc, var) *(V *)(&(loc)) = var
#define DVK(var, val) static const V var = { val, val, val, val }

#define VTR4(r0, r1, r2, r3)				\
{							\
     V _t0, _t1, _t2, _t3;				\
							\
     _t0 = __builtin_ia32_shufps((r0), (r1), 0x44);	\
     _t2 = __builtin_ia32_shufps((r0), (r1), 0xEE);	\
     _t1 = __builtin_ia32_shufps((r2), (r3), 0x44);	\
     _t3 = __builtin_ia32_shufps((r2), (r3), 0xEE);	\
							\
     r0 = __builtin_ia32_shufps(_t0, _t1, 0x88);	\
     r1 = __builtin_ia32_shufps(_t0, _t1, 0xDD);	\
     r2 = __builtin_ia32_shufps(_t2, _t3, 0x88);	\
     r3 = __builtin_ia32_shufps(_t2, _t3, 0xDD);	\
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
#define DVK(var, val) V var = _mm_set_ps1(val)

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
#define VFMS(a, b, c) VSUB(VMUL(a, b), c)
#define VFNMS(a, b, c) VSUB(c, VMUL(a, b))

#define VTW(op, x) {op, 0, x}, {op, 1, x}, {op, 2, x}, {op, 3, x}
