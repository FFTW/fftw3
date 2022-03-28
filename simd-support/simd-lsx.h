/*
 * Copyright (c) 2022 Loongson Technology Corporation Limited
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
#error "LSX only works in single/double precision"
#endif

#if defined(__GNUC__) && !defined(__loongarch_sx)
#error "compiling simd-lsx.h requires -mlsx"
#endif

#include <lsxintrin.h>

#ifdef FFTW_SINGLE
#define DS(d,s) s /* single-precision option */
#else
#define DS(d,s) d /* double-precision option */
#endif

#define SIMD_SUFFIX _lsx  /* for renaming */
#define VL DS(1,2)         /* SIMD vector length, in term of complex numbers */
#define SIMD_VSTRIDE_OKA(x) DS(SIMD_STRIDE_OKA(x),((x) == 2))
#define SIMD_STRIDE_OKPAIR SIMD_STRIDE_OK

typedef DS(__m128d, __m128) V;
#define VXOR(a, b) (V)__lsx_vxor_v((__m128i)a, (__m128i)b)
#define SHUFVALS(fp0,fp1,fp2,fp3) (((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | ((fp0)))
#define LDK(x) x

#ifdef FFTW_SINGLE
#define VADD(a, b)    (V)__lsx_vfadd_s((__m128)a, (__m128)b)
#define VSUB(a, b)    (V)__lsx_vfsub_s((__m128)a, (__m128)b)
#define VMUL(a, b)    (V)__lsx_vfmul_s((__m128)a, (__m128)b)
#define SHUF(a, b, c) (V)__lsx_vshuf4i_w((__m128i)a, c)
#define UNPCKL(a, b)  (V)__lsx_vilvl_w((__m128i)b, (__m128i)a)
#define UNPCKH(a, b)  (V)__lsx_vilvh_w((__m128i)b, (__m128i)a)
#define VDUPL(x)      SHUF(x, x, SHUFVALS(0, 0, 2, 2))
#define VDUPH(x)      SHUF(x, x, SHUFVALS(1, 1, 3, 3))
#define DVK(var, val) V var = {val, val, val, val}

#define STOREH(a, v)  __lsx_vstelm_d(v, a, 0, 1)
#define STOREL(a, v)  __lsx_vstelm_d(v, a, 0, 0)
#else
#define VADD(a, b)    (V)__lsx_vfadd_d((__m128d)a, (__m128d)b)
#define VSUB(a, b)    (V)__lsx_vfsub_d((__m128d)a, (__m128d)b)
#define VMUL(a, b)    (V)__lsx_vfmul_d((__m128d)a, (__m128d)b)
#define SHUF(a, b, c) (V)__lsx_vshuf4i_d((__m128i)a, (__m128i)b, c)
#define UNPCKL(a, b)  (V)__lsx_vilvl_d((__m128i)b, (__m128i)a)
#define UNPCKH(a, b)  (V)__lsx_vilvh_d((__m128i)b, (__m128i)a)
#define VDUPL(x)      UNPCKL(x, x)
#define VDUPH(x)      UNPCKH(x, x)
#define DVK(var, val) V var = {val, val}

#define STOREH(a, v)  __lsx_vstelm_d(v, a, 0, 1)
#define STOREL(a, v)  __lsx_vstelm_d(v, a, 0, 0)
#endif /* FFTW_SINGLE */

static inline V LDA(const R *x, INT ivs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */
  (void)ivs; /* UNUSED */
  return *(const V *)x;
}

static inline void STA(R *x, V v, INT ovs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */
  (void)ovs; /* UNUSED */
  *(V *)x = v;
}

#ifdef FFTW_SINGLE
static inline V LD(const R *x, INT ivs, const R *aligned_like)
{
  V var;
  (void)aligned_like; /* UNUSED */

  ((double *)&var)[0] = *((double *)x);
  ((double *)&var)[1] = *((double *)(x+ivs));

  return var;
}

static inline void ST(R *x, V v, INT ovs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */
  /* WARNING: the extra_iter hack depends upon STOREL occurring
     after STOREH */
  STOREH(x + ovs, v);
  STOREL(x, v);
}

#define STM2 ST
#define STN2(x, v0, v1, ovs) /* nop */
#define STM4(x, v, ovs, aligned_like) /* no-op */
#define STN4(x, v0, v1, v2, v3, ovs)			  \
{							  \
  V xxx0, xxx1, xxx2, xxx3;				  \
  xxx0 = UNPCKL(v0, v2);				  \
  xxx1 = UNPCKH(v0, v2);				  \
  xxx2 = UNPCKL(v1, v3);				  \
  xxx3 = UNPCKH(v1, v3);				  \
  STA(x, UNPCKL(xxx0, xxx2), 0, 0);			  \
  STA(x + ovs, UNPCKH(xxx0, xxx2), 0, 0);		  \
  STA(x + 2 * ovs, UNPCKL(xxx1, xxx3), 0, 0);	          \
  STA(x + 3 * ovs, UNPCKH(xxx1, xxx3), 0, 0);	          \
}

#else /* ! FFTW_SINGLE */
#define LD LDA
#define ST STA

#define STM2 STA
#define STN2(x, v0, v1, ovs) /* nop */
static inline void STM4(R *x, V v, INT ovs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */
  STOREL(x, v);
  STOREH(x + ovs, v);
}
#define STN4(x, v0, v1, v2, v3, ovs) /* nothing */
#endif

static inline V FLIP_RI(V x)
{
  return SHUF(x, x, SHUFVALS(1, 0, 3, 2));
}

static inline V VCONJ(V x)
{
  /* This will produce -0.0f (or -0.0d) even on broken
     compilers that do not distinguish +0.0 from -0.0.
     I bet some are still around. */
  union uvec {
    unsigned u[4];
    V v;
  };
  /* it looks like gcc-3.3.5 produces slow code unless PM is
     declared static. */
  static const union uvec pm = {
#ifdef FFTW_SINGLE
  { 0x00000000, 0x80000000, 0x00000000, 0x80000000 }
#else
  { 0x00000000, 0x00000000, 0x00000000, 0x80000000 }
#endif
  };
  return VXOR(pm.v, x);
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
#define VTW1(v, x)  \
  {TW_COS, v, x}, {TW_COS, v + 1, x}, {TW_SIN, v, x}, {TW_SIN, v + 1, x}
static inline V BYTW1(const R *t, V sr)
{
  const V *twp = (const V *)t;
  V tx = twp[0];
  V tr = UNPCKL(tx, tx);
  V ti = UNPCKH(tx, tx);
  tr = VMUL(tr, sr);
  sr = VBYI(sr);
  return VFMA(ti, sr, tr);
}
static inline V BYTWJ1(const R *t, V sr)
{
  const V *twp = (const V *)t;
  V tx = twp[0];
  V tr = UNPCKL(tx, tx);
  V ti = UNPCKH(tx, tx);
  tr = VMUL(tr, sr);
  sr = VBYI(sr);
  return VFNMS(ti, sr, tr);
}
#else /* !FFTW_SINGLE */
#define VTW1(v, x) {TW_CEXP, v, x}
static inline V BYTW1(const R *t, V sr)
{
  V tx = LD(t, 1, t);
  return VZMUL(tx, sr);
}
static inline V BYTWJ1(const R *t, V sr)
{
  V tx = LD(t, 1, t);
  return VZMULJ(tx, sr);
}
#endif
#define TWVL1 (VL)

/* twiddle storage #2: twice the space, faster (when in cache) */
#ifdef FFTW_SINGLE
#define VTW2(v, x)							  \
  {TW_COS, v, x}, {TW_COS, v, x}, {TW_COS, v + 1, x}, {TW_COS, v + 1, x}, \
  {TW_SIN, v, -x}, {TW_SIN, v, x}, {TW_SIN, v + 1, -x}, {TW_SIN, v + 1, x}
#else /* !FFTW_SINGLE */
#define VTW2(v, x)							  \
  {TW_COS, v, x}, {TW_COS, v, x}, {TW_SIN, v, -x}, {TW_SIN, v, x}
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
#ifdef FFTW_SINGLE
#define VTW3(v, x) {TW_CEXP, v, x}, {TW_CEXP, v + 1, x}
#define TWVL3 (VL)
#else
#define VTW3(v, x) VTW1(v, x)
#define TWVL3 TWVL1
#endif

/* twiddle storage for split arrays */
#ifdef FFTW_SINGLE
#define VTWS(v, x)                                                             \
  {TW_COS, v, x}, {TW_COS, v + 1, x}, {TW_COS, v + 2, x}, {TW_COS, v + 3, x},  \
  {TW_SIN, v, x}, {TW_SIN, v + 1, x}, {TW_SIN, v + 2, x}, {TW_SIN, v + 3, x}
#else
#define VTWS(v, x)                                                             \
  {TW_COS, v, x}, {TW_COS, v + 1, x}, {TW_SIN, v, x}, {TW_SIN, v + 1, x}
#endif
#define TWVLS (2 * VL)

#define VLEAVE() /* nothing */

#include "simd-common.h"
