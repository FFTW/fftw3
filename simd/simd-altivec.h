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
#error "Altivec only works in single precision"
#endif

#define VL 4            /* SIMD vector length */
#define ALIGNMENT 16

#if defined(__GNUC__) && (defined(__powerpc__) || defined(__ppc__))

typedef float __attribute__((aligned(16), vector_size(16))) V;
typedef int Vint __attribute__((vector_size(16)));

#define VADD(a, b) __builtin_altivec_vaddfp(a, b)
#define VSUB(a, b) __builtin_altivec_vsubfp(a, b)
#define VFMA(a, b, c) __builtin_altivec_vmaddfp(a, b, c)
#define VFNMS(a, b, c)__builtin_altivec_vnmsubfp(a, b, c) 

#define LD(var, loc) var = *(const V *)(&(loc))
#define ST(loc, var) *(V *)(&(loc)) = var
#define DVK(var, val) static const V var = { val, val, val, val }

static inline V VMUL(V a, V b)
{
     DVK(zero, 0.0);
     return VFMA(a, b, zero);
}

static inline V VMRGL(V a, V b)
{
     V ret;
     __asm__("vmrglw %0, %1, %2" : "=v"(ret) : "v"(a), "v"(b));
     return ret;
}

static inline V VMRGH(V a, V b)
{
     V ret;
     __asm__("vmrghw %0, %1, %2" : "=v"(ret) : "v"(a), "v"(b));
     return ret;
}


#define ST4(a, ovs, r0, r1, r2, r3)			\
{							\
     R *_b = &(a);					\
     V _t0, _t1, _t2, _t3;				\
     _t0 = VMRGH(r0, r2);				\
     _t1 = VMRGH(r1, r3);				\
     _t2 = VMRGL(r0, r2);				\
     _t3 = VMRGL(r1, r3);				\
							\
     ST(_b[0 * ovs], VMRGH(_t0,_t1));			\
     ST(_b[1 * ovs], VMRGL(_t0,_t1));			\
     ST(_b[2 * ovs], VMRGH(_t2,_t3));			\
     ST(_b[3 * ovs], VMRGL(_t2,_t3));			\
}

#endif

#define VTW(op, x) {op, 0, x}, {op, 1, x}, {op, 2, x}, {op, 3, x}
