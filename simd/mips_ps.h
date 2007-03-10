/*
 * Copyright (c) 2007 Codesourcery
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

/* Low-level macros and functions for the MIPS paired-single SIMD
   instructions. */

#ifndef _SIMD_MMDX_H
#define _SIMD_MMDX_H

#include <endian.h>


#if BYTE_ORDER == BIG_ENDIAN
#define  USE_BIG_ENDIAN 1
#else
#define  USE_BIG_ENDIAN 0
#endif


/* Macros/functions are defined in two ways.
    - Using SIMD intrinsics and vector operations, if the supported by
      the compiler (as indicated by the __mips_paired_single_float
      macro).  MIPS SIMD intrinsics and vector operations are available
      in GCC versions 4.1 and later when the -mpaired-single command-line
      option is given.
    - Using inline assembly.
*/
#if defined(__mips_paired_single_float)
#  define USE_SIMD_INTRINSICS  1
#  define USE_INLINE_FUNCTIONS 0
#else
#  define USE_SIMD_INTRINSICS  0
#  define USE_INLINE_FUNCTIONS 0
#endif


#if USE_SIMD_INTRINSICS

typedef float psingle __attribute__ ((vector_size(8)));

#define _mips64_alu_op2(a,b,op,asm_op) (a op b)

#define _mips64_alu_op3(a,b,c,s,op,asm_op) (s(a * b op c))

#else /* !USE_SIMD_INTRINSICS */

typedef float psingle __attribute__ ((mode (DF)));

#define _mips64_alu_op2(a,b,op,asm_op) \
({\
  psingle r; \
  __asm__ (asm_op " %0,%1,%2" : "=f"(r) : "f"(a), "f"(b)); \
  r; \
})

#define _mips64_alu_op3(a,b,c,s,op,asm_op) \
({\
  psingle r; \
  __asm__ (asm_op " %0,%1,%2,%3" : "=f"(r) : "f"(c), "f"(a), "f"(b)); \
  r; \
})

#endif /* USE_SIMD_INTRINSICS */



/* Wrap operations into static inline function. */
#define _mips64_alu_op2_fn(fn,op,asm_op) \
static inline psingle _mips64_##fn(psingle a, psingle b) \
{ \
  return _mips64_alu_op2(a,b,op,asm_op); \
}

#define _mips64_alu_op3_fn(fn,s,op,asm_op) \
static inline psingle _mips64_##fn(psingle a, psingle b, psingle c) \
{ \
  return _mips64_alu_op3(a,b,c,s,op,asm_op); \
}



/***************************************************************************
* Paired-single load/store.
****************************************************************************/

#if USE_SIMD_INTRINSICS
#define _mips64_ld(x) \
({ \
  psingle reg; \
  reg = *x; \
  reg; \
})

#define _mips64_st(x,v) \
({ \
  *(psingle*)(x) = v;\
})

#else /* !USE_SIMD_INTRINSICS */

#define _mips64_ld(x) \
({ \
  psingle reg; \
  __asm__ ("ldc1 %0,%1" : "=f"(reg) : "m"(*x)); \
  reg; \
})

#define _mips64_st(x,v) \
({ \
  psingle reg = v; \
  __asm__ ("sdc1 %1,%0" : "=m"(*x) : "f"(reg)); \
})

#endif /* USE_SIMD_INTRINSICS */



/***************************************************************************
* Paired-single arithmetic operations.
****************************************************************************/

#if USE_INLINE_FUNCTIONS
  _mips64_alu_op2_fn(add,+,"add.ps")
  _mips64_alu_op2_fn(sub,-,"sub.ps")
  _mips64_alu_op2_fn(mul,*,"mul.ps")
  _mips64_alu_op3_fn(madd,,+,"madd.ps")
  _mips64_alu_op3_fn(msub,,-,"msub.ps")
  _mips64_alu_op3_fn(nmsub,-,-,"nmsub.ps")
#else
#  define _mips64_add(a,b)     _mips64_alu_op2(a,b,+,"add.ps")
#  define _mips64_sub(a,b)     _mips64_alu_op2(a,b,-,"sub.ps")
#  define _mips64_mul(a,b)     _mips64_alu_op2(a,b,*,"mul.ps")
#  define _mips64_madd(a,b,c)  _mips64_alu_op3(a,b,c,,+,"madd.ps")
#  define _mips64_msub(a,b,c)  _mips64_alu_op3(a,b,c,,-,"msub.ps")
#  define _mips64_nmsub(a,b,c) _mips64_alu_op3(a,b,c,-,-,"nmsub.ps")
#endif



/***************************************************************************
* Paired-single packing, upper/lower store, shuffle, load scalar, and
* change-sign.
****************************************************************************/

#if USE_SIMD_INTRINSICS

#if USE_BIG_ENDIAN
#  define _mips64_pl(a,b)      __builtin_mips_puu_ps(a,b)
#  define _mips64_pu(a,b)      __builtin_mips_pll_ps(a,b)
#else
#  define _mips64_pl(a,b)      __builtin_mips_pll_ps(a,b)
#  define _mips64_pu(a,b)      __builtin_mips_puu_ps(a,b)
#endif

static inline psingle _mips64_sl(float *a, psingle b)
{
#if USE_BIG_ENDIAN
  float b_lo = __builtin_mips_cvt_s_pu(b);
#else
  float b_lo = __builtin_mips_cvt_s_pl(b);
#endif
  *a = b_lo;
}
static inline psingle _mips64_su(float *a,psingle b)
{
#if USE_BIG_ENDIAN
  float b_hi = __builtin_mips_cvt_s_pl(b);
#else
  float b_hi = __builtin_mips_cvt_s_pu(b);
#endif
  *a = b_hi;
}

static inline psingle _mips64_shuffle(psingle a)
{
  psingle res;

  res = __builtin_mips_plu_ps(a,a);
  return res;
}

#define _mips64_lc(c) \
({ \
  psingle v = {c,c};\
  v; \
})

static inline psingle _mips64_chs_i(psingle v)
{
#if USE_BIG_ENDIAN
  static psingle change_sign = {1.0, -1.0};
#else
  static psingle change_sign = {-1.0, 1.0};
#endif

  v = v * change_sign;

  return v;
}

#else /* !USE_SIMD_INTRINSICS */

#if USE_BIG_ENDIAN
#  define _mips64_pl(a,b)      _mips64_alu_op2(a,b,,"puu.ps")
#  define _mips64_pu(a,b)      _mips64_alu_op2(a,b,,"pll.ps")
#else
#  define _mips64_pl(a,b)      _mips64_alu_op2(a,b,,"pll.ps")
#  define _mips64_pu(a,b)      _mips64_alu_op2(a,b,,"puu.ps")
#endif


#define _mips64_sl(a,b) \
({ \
  float b_lo; \
  __asm__ ("cvt.s.pu %0,%1"    : "=f"(b_lo) : "f"(b)); \
  *(a) = b_lo; \
})

#define _mips64_su(a,b) \
({ \
  float b_up; \
  __asm__ ("cvt.s.pl %0,%1"    : "=f"(b_up) : "f"(b)); \
  *(a) = b_up; \
})

#define _mips64_shuffle(v) \
({ \
  psingle res; \
  __asm__("plu.ps %0,%1,%2" : "=f"(res) : "f"(v), "f"(v)); \
  res; \
})

#define _mips64_lc(c) \
({ \
  V r; \
  float c_f=c; \
  __asm__ ("cvt.ps.s %0,%1,%2" : "=f"(r) : "f"(c_f), "f"(c_f)); \
  r; \
})

typedef union
{
  float f[2];
  psingle ps;
} pstof;

#define _mips64_chs_i(v) \
({ \
  static pstof my_pstof = { {1.0, -1.0} }; \
  psingle r; \
  __asm__("mul.ps %0,%1,%2" : "=f"(r) : "f"(v), "f"(my_pstof.ps)); \
  r; \
})

#endif /* USE_SIMD_INTRINSICS */

#endif /* _SIMD_MMDX_H */
