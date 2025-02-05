/*
 * Copyright (c) 2003, 2007-14 Matteo Frigo
 * Copyright (c) 2003, 2007-14 Massachusetts Institute of Technology
 *
 * Contributed by Shiyou Yin <yinshiyou-hf@loongson.cn>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#if defined(FFTW_LDOUBLE) || defined(FFTW_QUAD)
#error "LASX only works in single/double precision"
#endif

#if defined(__GNUC__) && !defined(__loongarch_asx)
#error "compiling simd-lasx.h requires -mlasx"
#endif

#include <lasxintrin.h>

#ifdef FFTW_SINGLE
#define DS(d,s) s /* single-precision option */
#else
#define DS(d,s) d /* double-precision option */
#endif

#define SIMD_SUFFIX _lasx  /* for renaming */
#define VL DS(2,4)         /* SIMD vector length, in term of complex numbers */
#define SIMD_VSTRIDE_OKA(x) ((x) == 2)
#define SIMD_STRIDE_OKPAIR SIMD_STRIDE_OK

typedef DS(__m256d, __m256) V;
#define VXOR(a, b) (V)__lasx_xvxor_v((__m256i)a, (__m256i)b)
#define LDK(x) x
#define SHUFVALD(fp0,fp1) (((fp1) << 3) | ((fp0) << 2) | ((fp1) << 1) | ((fp0)))
#define SHUFVALS(fp0,fp1,fp2,fp3) (((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | ((fp0)))

#ifdef FFTW_SINGLE
#define VADD(a, b)    (V)__lasx_xvfadd_s((__m256)a, (__m256)b)
#define VSUB(a, b)    (V)__lasx_xvfsub_s((__m256)a, (__m256)b)
#define VMUL(a, b)    (V)__lasx_xvfmul_s((__m256)a, (__m256)b)
#define SHUF(a, b, c) (V)__lasx_xvshuf4i_w((__m256i)a, c)
#define UNPCKL(a, b)  (V)__lasx_xvilvl_w((__m256i)b, (__m256i)a)
#define UNPCKH(a, b)  (V)__lasx_xvilvh_w((__m256i)b, (__m256i)a)
#define VDUPL(x) SHUF(x, x, SHUFVALS(0, 0, 2, 2))
#define VDUPH(x) SHUF(x, x, SHUFVALS(1, 1, 3, 3))

#define STOREH1(a, v) __lasx_xvstelm_d(v, a, 0, 3)
#define STOREL1(a, v) __lasx_xvstelm_d(v, a, 0, 2)
#define STOREH0(a, v) __lasx_xvstelm_d(v, a, 0, 1)
#define STOREL0(a, v) __lasx_xvstelm_d(v, a, 0, 0)
//#define DVK(var, val) V var = (V)__lasx_xvreplgr2vr_w(val)
#define DVK(var, val) V var = {val,val,val,val,val,val,val,val}
#else
#define VADD(a, b)    (V)__lasx_xvfadd_d((__m256d)a, (__m256d)b)
#define VSUB(a, b)    (V)__lasx_xvfsub_d((__m256d)a, (__m256d)b)
#define VMUL(a, b)    (V)__lasx_xvfmul_d((__m256d)a, (__m256d)b)
#define SHUF(a, b, c) (V)__lasx_xvshuf4i_d((__m256i)a, (__m256i)b, c) /* each index is 2-bits in c */
#define UNPCKL(a, b)  (V)__lasx_xvilvl_d((__m256i)b, (__m256i)a)
#define UNPCKH(a, b)  (V)__lasx_xvilvh_d((__m256i)b, (__m256i)a)
#define VDUPL(x) UNPCKL(x, x)
#define VDUPH(x) UNPCKH(x, x)

#define STOREH1(a, v) __lasx_xvstelm_d(v, a, 0, 3)
#define STOREL1(a, v) __lasx_xvstelm_d(v, a, 0, 2)
#define STOREH0(a, v) __lasx_xvstelm_d(v, a, 0, 1)
#define STOREL0(a, v) __lasx_xvstelm_d(v, a, 0, 0)
//#define DVK(var, val) V var = (V)__lasx_xvreplgr2vr_d(val)
#define DVK(var, val) V var = {val, val, val, val}
#endif /* FFTW_SINGLE */

static inline V LDA(const R *x, INT ivs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */
  (void)ivs; /* UNUSED */
  return (V)__lasx_xvld(x, 0);
}

static inline void STA(R *x, V v, INT ovs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */
  (void)ovs; /* UNUSED */
  __lasx_xvst((__m256i)v, x, 0);
}

#ifdef FFTW_SINGLE
static inline V LD(const R *x, INT ivs, const R *aligned_like)
{
  V var;
  (void)aligned_like; /* UNUSED */

  ((double *)&var)[0] = *((double *)x);
  ((double *)&var)[1] = *((double *)(x + ivs));
  ((double *)&var)[2] = *((double *)(x + 2*ivs));
  ((double *)&var)[3] = *((double *)(x + 3*ivs));

  return var;
}

static inline void ST(R *x, V v, INT ovs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */
  /* WARNING: the extra_iter hack depends upon STOREL occurring
     after STOREH */
  STOREH1(x + 3*ovs, v);
  STOREL1(x + 2*ovs, v);
  STOREH0(x + ovs, v);
  STOREL0(x, v);
}

#define STM2(x, v, ovs, aligned_like) /* no-op */
static inline void STN2(R *x, V v0, V v1, INT ovs)
{
    R *x1 = x + ovs;
    R *x2 = x1 + ovs;
    R *x3 = x2 + ovs;

    __lasx_xvstelm_d(v0, x,   0, 0);
    __lasx_xvstelm_d(v1, x,   8, 0);
    __lasx_xvstelm_d(v0, x1,  0, 1);
    __lasx_xvstelm_d(v1, x1,  8, 1);
    __lasx_xvstelm_d(v0, x2,  0, 2);
    __lasx_xvstelm_d(v1, x2,  8, 2);
    __lasx_xvstelm_d(v0, x3,  0, 3);
    __lasx_xvstelm_d(v1, x3,  8, 3);
}

#define STM4(x, v, ovs, aligned_like) /* no-op */
static inline void STN4(R *x, V v0, V v1, V v2, V v3, INT ovs)
{
    R *x1 = x + ovs;
    R *x2 = x1 + ovs;
    R *x3 = x2 + ovs;
    R *x4 = x3 + ovs;
    R *x5 = x4 + ovs;
    R *x6 = x5 + ovs;
    R *x7 = x6 + ovs;
    V t0, t1, t2, t3;

    t0 = UNPCKL(v0, v1);
    t1 = UNPCKH(v0, v1);
    t2 = UNPCKL(v2, v3);
    t3 = UNPCKH(v2, v3);
    __lasx_xvstelm_d(t0, x,    0, 0);
    __lasx_xvstelm_d(t2, x,    8, 0);
    __lasx_xvstelm_d(t0, x1,   0, 1);
    __lasx_xvstelm_d(t2, x1,   8, 1);
    __lasx_xvstelm_d(t1, x2,   0, 0);
    __lasx_xvstelm_d(t3, x2,   8, 0);
    __lasx_xvstelm_d(t1, x3,   0, 1);
    __lasx_xvstelm_d(t3, x3,   8, 1);
    __lasx_xvstelm_d(t0, x4,   0, 2);
    __lasx_xvstelm_d(t2, x4,   8, 2);
    __lasx_xvstelm_d(t0, x5,   0, 3);
    __lasx_xvstelm_d(t2, x5,   8, 3);
    __lasx_xvstelm_d(t1, x6,   0, 2);
    __lasx_xvstelm_d(t3, x6,   8, 2);
    __lasx_xvstelm_d(t1, x7,   0, 3);
    __lasx_xvstelm_d(t3, x7,   8, 3);
}

#else
/* !FFTW_SINGLE */
static inline V LD(const R *x, INT ivs, const R *aligned_like)
{
  V var;
  (void)aligned_like; /* UNUSED */

  var[0] = x[0];
  var[1] = x[1];
  var[2] = x[ivs];
  var[3] = x[ivs +1];

  return var;
}

static inline void ST(R *x, V v, INT ovs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */

  STOREH1(x + ovs + 1, v);
  STOREL1(x + ovs, v);
  STOREH0(x + 1, v);
  STOREL0(x, v);
}

#define STM2 ST
#define STN2(x, v0, v1, ovs) /* nop */

#define STM4(x, v, ovs, aligned_like) /* no-op */
static inline void STN4(R *x, V v0, V v1, V v2, V v3, INT ovs)
{
    R *x1 = x + ovs;
    R *x2 = x1 + ovs;
    R *x3 = x2 + ovs;

    __lasx_xvstelm_d(v0, x,   0, 0);
    __lasx_xvstelm_d(v1, x,   8, 0);
    __lasx_xvstelm_d(v2, x,  16, 0);
    __lasx_xvstelm_d(v3, x,  24, 0);
    __lasx_xvstelm_d(v0, x1,  0, 1);
    __lasx_xvstelm_d(v1, x1,  8, 1);
    __lasx_xvstelm_d(v2, x1, 16, 1);
    __lasx_xvstelm_d(v3, x1, 24, 1);
    __lasx_xvstelm_d(v0, x2,  0, 2);
    __lasx_xvstelm_d(v1, x2,  8, 2);
    __lasx_xvstelm_d(v2, x2, 16, 2);
    __lasx_xvstelm_d(v3, x2, 24, 2);
    __lasx_xvstelm_d(v0, x3,  0, 3);
    __lasx_xvstelm_d(v1, x3,  8, 3);
    __lasx_xvstelm_d(v2, x3, 16, 3);
    __lasx_xvstelm_d(v3, x3, 24, 3);
}
#endif

static inline V FLIP_RI(V x)
{
  return SHUF(x, x, SHUFVALS(1, 0, 3, 2));
}

static inline V VCONJ(V x)
{
  union uvec {
    unsigned u[8];
    V v;
  };
  static const union uvec pmpm = {
#ifdef FFTW_SINGLE
  { 0x00000000, 0x80000000, 0x00000000, 0x80000000,
    0x00000000, 0x80000000, 0x00000000, 0x80000000 }
#else
  { 0x00000000, 0x00000000, 0x00000000, 0x80000000,
    0x00000000, 0x00000000, 0x00000000, 0x80000000 }
#endif
  };
  return VXOR(pmpm.v, x);
}

static inline V VBYI(V x)
{
  return FLIP_RI(VCONJ(x));
}

/* FMA support */
#define VFMA(a, b, c)   VADD(c, VMUL(a, b))
#define VFNMS(a, b, c)  VSUB(c, VMUL(a, b))
#define VFMS(a, b, c)   VSUB(VMUL(a, b), c)
#define VFMAI(b, c)     VADD(c, VBYI(b))
#define VFNMSI(b, c)    VSUB(c, VBYI(b))
#define VFMACONJ(b, c)  VADD(VCONJ(b), c)
#define VFMSCONJ(b, c)  VSUB(VCONJ(b), c)
#define VFNMSCONJ(b, c) VSUB(c, VCONJ(b))

static inline V VZMUL(V tx, V sr)
{
  V tr = VDUPL(tx);
  V ti = VDUPH(tx);
  tr = VMUL(sr, tr);
  sr = VBYI(sr);
  return VFMA(ti, sr, tr);
}

static inline V VZMULJ(V tx, V sr)
{
  V tr = VDUPL(tx);
  V ti = VDUPH(tx);
  tr = VMUL(sr, tr);
  sr = VBYI(sr);
  return VFNMS(ti, sr, tr);
}

static inline V VZMULI(V tx, V sr)
{
  V tr = VDUPL(tx);
  V ti = VDUPH(tx);
  ti = VMUL(ti, sr);
  sr = VBYI(sr);
  return VFMS(tr, sr, ti);
}

static inline V VZMULIJ(V tx, V sr)
{
  V tr = VDUPL(tx);
  V ti = VDUPH(tx);
  ti = VMUL(ti, sr);
  sr = VBYI(sr);
  return VFMA(tr, sr, ti);
}

/* twiddle storage #1: compact, slower */
#ifdef FFTW_SINGLE
# define VTW1(v,x) {TW_CEXP, v, x}, {TW_CEXP, v+1, x}, {TW_CEXP, v+2, x}, {TW_CEXP, v+3, x}
#else
# define VTW1(v,x) {TW_CEXP, v, x}, {TW_CEXP, v+1, x}
#endif
#define TWVL1 (VL)

static inline V BYTW1(const R *t, V sr)
{
     return VZMUL(LDA(t, 2, t), sr);
}

static inline V BYTWJ1(const R *t, V sr)
{
     return VZMULJ(LDA(t, 2, t), sr);
}

/* twiddle storage #2: twice the space, faster (when in cache) */
#ifdef FFTW_SINGLE
# define VTW2(v,x)                                                      \
   {TW_COS, v, x}, {TW_COS, v, x}, {TW_COS, v+1, x}, {TW_COS, v+1, x},  \
   {TW_COS, v+2, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, {TW_COS, v+3, x}, \
   {TW_SIN, v, -x}, {TW_SIN, v, x}, {TW_SIN, v+1, -x}, {TW_SIN, v+1, x}, \
   {TW_SIN, v+2, -x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, -x}, {TW_SIN, v+3, x}
#else
# define VTW2(v,x)                                                      \
   {TW_COS, v, x}, {TW_COS, v, x}, {TW_COS, v+1, x}, {TW_COS, v+1, x},  \
   {TW_SIN, v, -x}, {TW_SIN, v, x}, {TW_SIN, v+1, -x}, {TW_SIN, v+1, x}
#endif
#define TWVL2 (2 * VL)

static inline V BYTW2(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V si = FLIP_RI(sr);
     V tr = twp[0], ti = twp[1];
     return VFMA(tr, sr, VMUL(ti, si));
}

static inline V BYTWJ2(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V si = FLIP_RI(sr);
     V tr = twp[0], ti = twp[1];
     return VFNMS(ti, si, VMUL(tr, sr));
}

/* twiddle storage #3 */
#define VTW3 VTW1
#define TWVL3 TWVL1

/* twiddle storage for split arrays */
#ifdef FFTW_SINGLE
# define VTWS(v,x)                                                      \
  {TW_COS, v, x}, {TW_COS, v+1, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, \
  {TW_COS, v+4, x}, {TW_COS, v+5, x}, {TW_COS, v+6, x}, {TW_COS, v+7, x}, \
  {TW_SIN, v, x}, {TW_SIN, v+1, x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, x}, \
  {TW_SIN, v+4, x}, {TW_SIN, v+5, x}, {TW_SIN, v+6, x}, {TW_SIN, v+7, x}
#else
# define VTWS(v,x)                                                      \
  {TW_COS, v, x}, {TW_COS, v+1, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, \
  {TW_SIN, v, x}, {TW_SIN, v+1, x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, x}
#endif
#define TWVLS (2 * VL)

#define VLEAVE() /* nothing */

#include "simd-common.h"
