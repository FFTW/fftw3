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

#ifndef _SIMD_SIMD_MIPS_PS_H
#define _SIMD_SIMD_MIPS_PS_H

#ifndef FFTW_SINGLE
#error "MIPS PS only works in single precision"
#endif

#define VL 1            /* SIMD complex vector length */
#define ALIGNMENT 8     /* alignment for LD/ST */
#define ALIGNMENTA 8    /* alignment for LDA/STA */
#define SIMD_VSTRIDE_OKA(x) 1
#define SIMD_STRIDE_OKPAIR SIMD_STRIDE_OKA

/* Codelets allows stores to be performed in one of two ways:
   - Values can be written back as they are generated.  This potentially
     lowers register pressure by freeing the register holding the value
     for reuse.
   - Values can be written back in groups to consecutive addresses.
     This potentially lowers the cost of address generation.
*/
#define USE_MULTIPLE_STORES 0

#define RIGHT_CPU X(have_mips_ps)
extern int RIGHT_CPU(void);

#if defined(__mips__)

#include <mips_ps.h>

typedef psingle V;

#define LDK(x) x
#define DVK(var, val) \
  const V var = _mips64_lc(val)

#define VADD     _mips64_add
#define VSUB     _mips64_sub
#define VMUL     _mips64_mul
#define VFMS     _mips64_msub
#define VFMA     _mips64_madd
#define VFNMS    _mips64_nmsub

#define UNPCKL   _mips64_pl
#define UNPCKH   _mips64_pu
#define STOREL   _mips64_sl
#define STOREH   _mips64_su
#define SHUFFLE  _mips64_shuffle

#define FLIP_RI  SHUFFLE
#define VCONJ    _mips64_chs_i

#define VFMACONJ(b,c)  VADD(VCONJ(b),c)
#define VFMSCONJ(b,c)  VSUB(VCONJ(b),c)
#define VFNMSCONJ(b,c) VSUB(c, VCONJ(b))

static inline V LDA(const R *x, INT ivs, const R *aligned_like)
{
  (void)aligned_like;
  (void)ivs;
  return _mips64_ld((V*)x);
}
#define LD LDA

static inline void STA(R *x, V v, INT ovs, const R *aligned_like)
{
  (void)aligned_like;
  (void)ovs;
  _mips64_st(x, v);
}
#define ST STA

#if USE_MULTIPLE_STORES
#  define STM2(x,v,ovs,a) /* nop */

static inline void STN2(R *x, V v0, V v1, INT ovs)
{
  (void)ovs;
  STA(x,   v0, 0, 0);
  STA(x+2, v1, 0, 0);
}
#else /* !USE_MULTIPLE_STORES */
#  define STM2 STA
#  define STN2(x,v0,v1,ovs) /* nop */
#endif

#if USE_MULTIPLE_STORES
#  define STM4(x,v,ovs,a) /* nop */

static inline void STN4(R *x, V v0, V v1, V v2, V v3, INT ovs)
{
     R *x_ptr = x;
     STA(x_ptr, UNPCKL(v0,v1), 0, 0); x_ptr += ovs;
     STA(x_ptr, UNPCKL(v2,v3), 0, 0); x_ptr += ovs;
     STA(x_ptr, UNPCKH(v0,v1), 0, 0); x_ptr += ovs;
     STA(x_ptr, UNPCKH(v2,v3), 0, 0);
}
#else /* !USE_MULTIPLE_STORES */
static inline void STM4(R *x, V v, INT ovs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     STOREL(x, v);
     STOREH(x + ovs, v);
}
#  define STN4(x, v0, v1, v2, v3, ovs) /* nop */
#endif


static inline V VBYI(V x)
{
     x = VCONJ(x);
     x = FLIP_RI(x);
     return x;
}

#define VFMAI(b, c)  VADD(c, VBYI(b))
#define VFNMSI(b, c) VSUB(c, VBYI(b))

static inline V VZMUL(V tx, V sr)
{
     V tr = UNPCKL(tx, tx);
     V ti = UNPCKH(tx, tx);
     tr = VMUL(sr, tr);
     sr = VBYI(sr);
     return VFMA(ti,sr,tr);
}

static inline V VZMULJ(V tx, V sr)
{
     V tr = UNPCKL(tx, tx);
     V ti = UNPCKH(tx, tx);
     tr = VMUL(sr, tr);
     sr = VBYI(sr);
     return VFNMS(ti,sr,tr);
}

/* twiddle storage #1: compact, slower */
#define VTW1(v,x) {TW_CEXP, v, x}
#define TWVL1 1

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

/* twiddle storage #2: twice the space, faster (when in cache) */
#define VTW2(v,x)							\
  {TW_COS, v, x}, {TW_COS, v, x}, {TW_SIN, v, -x}, {TW_SIN, v, x}
#define TWVL2 2

static inline V BYTW2(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V si = FLIP_RI(sr);
     V tr = twp[0], ti = twp[1];
     return VFMA(tr,sr,VMUL(ti,si));
}

static inline V BYTWJ2(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V si = FLIP_RI(sr);
     V tr = twp[0], ti = twp[1];
     return VFNMS(ti,si, VMUL(tr,sr));
}

static inline V VZMULI(V tx, V sr)
{
     V tr = UNPCKL(tx, tx);
     V ti = UNPCKH(tx, tx);
     ti = VMUL(ti, sr);
     sr = VBYI(sr);
     return VFMS(tr,sr,ti);
}

static inline V VZMULIJ(V tx, V sr)
{
     V tr = UNPCKL(tx, tx);
     V ti = UNPCKH(tx, tx);
     ti = VMUL(ti, sr);
     sr = VBYI(sr);
     return VFMA(tr,sr,ti);
}

/* twiddle storage #3 */
#define VTW3(v,x) VTW1(v,x)
#define TWVL3 TWVL1

/* twiddle storage for split arrays */
#define VTWS(v,x)							\
  {TW_COS, v, x}, {TW_COS, v+1, x}, {TW_SIN, v, x}, {TW_SIN, v+1, x}
#define TWVLS (2 * VL)


#endif /* defined(__mips__) */

#endif /* _SIMD_SIMD_MIPS_PS_H */
