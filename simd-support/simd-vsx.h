/*
 * Copyright (c) 2003, 2007-14 Matteo Frigo
 * Copyright (c) 2003, 2007-14 Massachusetts Institute of Technology
 *
 * VSX SIMD implementation added 2015 Erik Lindahl.
 * Erik Lindahl places his modifications in the public domain.
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
#  error "VSX only works in single or double precision"
#endif

#ifdef FFTW_SINGLE
#  define DS(d,s) s /* single-precision option */
#  define SUFF(name) name ## s
#else
#  define DS(d,s) d /* double-precision option */
#  define SUFF(name) name ## d
#endif

#define SIMD_SUFFIX  _vsx  /* for renaming */
#define VL DS(1,2)         /* SIMD vector length, in term of complex numbers */
#define SIMD_VSTRIDE_OKA(x) DS(1,((x) == 2))
#define SIMD_STRIDE_OKPAIR SIMD_STRIDE_OK

#include <altivec.h>
#include <stdio.h>

typedef DS(vector double,vector float) V;

#define VADD(a,b)   vec_add(a,b)
#define VSUB(a,b)   vec_sub(a,b)
#define VMUL(a,b)   vec_mul(a,b)
#define VXOR(a,b)   vec_xor(a,b)
#define UNPCKL(a,b) vec_mergel(a,b)
#define UNPCKH(a,b) vec_mergeh(a,b)
#define VDUPL(a)    DS(vsx_permdi(a,a,3),vsx_mergeo(a,a))
#define VDUPH(a)    DS(vsx_permdi(a,a,0),vsx_mergee(a,a))

/* We use inline asm for a handful of special instructions to make sure this 
 * file keeps working although the compilers are still a bit immature and 
 * not entirely compatible for VSX:
 *
 * - vmrgew/vmrgow is not supported with just -mvsx, but you need a special 
 *   -mcpu=power8 flag for gcc, and it is anyway only implemented for integer
 *   data. To avoid hassle, we roll our own.
 * -  vpermxor is a very special instruction intended for raid calculations, 
 *   but since it helps us save an operation in VBYI(), we have picked it up 
 *   (there is no intrinsic for it in gcc at least).
 * - The VSX instruction set is a bit peculiar since Power8 normally runs in
 *   little-endian memory mode, but the registers are still organized as big
 *   endian. To handle this, many memory operations are implemented as other
 *   instructions combined with permutations. Since we anyway need to permute
 *   some load/stores, we can save two (!) permutations by using the
 *   lower-level instructions directly in a few cases.
 * - The xxpermdi intrinsic has slightly different naming in gcc vs. xlc, and
 *   worse: They interpret bot the constant and order of parameters in opposite
 *   ways in little endian mode. We stick to the gcc version, and use inline
 *   asm for xlc.
 * - vec_neg() seems to be missing in gcc.
 * - the gcc versions of vec_vsx_ld cannot take a (double *) argument in
 *   gcc-4.9, but accepts (vector double *).
 * - xlc does not do word-swapping for little endian if we use vector pointers
 *   (but float/double pointers work), and the vec_xl() load function takes a
 *   non-constant pointer, so we need to cast that away.
 */

#ifdef __ibmxl__
#    define vsx_load(addr)        vec_xl(0,(DS(double,float) *)addr)
#    define vsx_store(a,addr)     vec_xst(a,0,addr)
#    define vsx_lxsdx(addr)       ({ V res;__asm__ ( "lxsdx %x0,0,%1" : "=v" (res) : "r" (addr)); res; })
#    define vsx_stxsdx(v,addr)    { __asm__ ( "stxsdx %x0,0,%1" :: "v" (v), "r" (addr) : "memory" ); }
#    define vsx_permdi(a,b,c)     ({ V res; __asm__ ( "xxpermdi %x0,%x1,%x2,%3" : "=v" (res) : "v" (a), "v" (b), "K" (c)); res; })
#    define vsx_neg(a)            vec_neg(a)
#else
#    define vsx_load(addr)        vec_vsx_ld(0,(V *)addr)
#    define vsx_store(a,addr)     vec_vsx_st(a,0,(V *)addr)
#    define vsx_lxsdx(addr)       ({ V res; __asm__ ( "lxsdx %0,0,%1" : "=ww" (res) : "r" (addr)); res; })
#    define vsx_stxsdx(v,addr)    { __asm__ ( "stxsdx %0,0,%1" :: "ww" (v), "r" (addr) : "memory" ); }
#    define vsx_permdi(a,b,c)     vec_xxpermdi(a,b,c)
#    define vsx_neg(a)            ({ V res; __asm__ ( DS("xvnegdp","xvnegsp") " %0,%1" : "=ww" (res) : "ww" (a)); res; })
#endif
/* These (altivec) intrinsics do not depend on the compiler */
#define vsx_vpermxor(a,b,c)       ({ V res; __asm__ ( "vpermxor %0,%1,%2,%3" : "=v" (res) : "v" (a), "v" (b), "v" (c)); res; })
#define vsx_mergee(a,b)           ({ V res; __asm__ ( "vmrgew %0,%1,%2" : "=v" (res) : "v" (a), "v" (b)); res; })
#define vsx_mergeo(a,b)           ({ V res; __asm__ ( "vmrgow %0,%1,%2" : "=v" (res) : "v" (a), "v" (b)); res; })

static inline V LDK(R f) { return vec_splats(f); }

#define DVK(var, val) const R var = K(val)

static inline V VCONJ(V x)
{
  const V pmpm = vec_mergel(vec_splats((R)0.0),vsx_neg(vec_splats((R)0.0)));
  return vec_xor(x, pmpm);
}

static inline V LDA(const R *x, INT ivs, const R *aligned_like)
{
  return vsx_load(x);
}

static inline void STA(R *x, V v, INT ovs, const R *aligned_like)
{
  vsx_store(v,x);
}

static inline V FLIP_RI(V x)
{
#ifdef FFTW_SINGLE
  const vector unsigned int perm = { 0x07060504, 0x03020100, 0x0f0e0d0c, 0x0b0a0908 };
  return vec_perm(x, x, (vector unsigned char)perm);
#else
  return vsx_permdi(x,x,2);
#endif
}

#ifdef FFTW_SINGLE

static inline V LD(const R *x, INT ivs, const R *aligned_like)
{
  return vsx_permdi(vsx_lxsdx(x+ivs),vsx_lxsdx(x),0);
}

static inline void ST(R *x, V v, INT ovs, const R *aligned_like)
{
  vsx_stxsdx(v,x+ovs);
  vsx_stxsdx(vsx_permdi(v,v,3),x);
}
#else
/* DOUBLE */

#  define LD LDA
#  define ST STA

#endif

#define STM2 DS(STA,ST)
#define STN2(x, v0, v1, ovs) /* nop */

#ifdef FFTW_SINGLE

#  define STM4(x, v, ovs, aligned_like) /* no-op */
/* Note on optimization for VSX: Normally the vec_vsx_st() store instructions would
 * be translated into a permute and two stxsdx on little endian. We bypass this
 * and access the lower-level instructions directly, which saves us four permutes.
 */
static inline void STN4(R *x, V v0, V v1, V v2, V v3, int ovs)
{
    V xxx0, xxx1, xxx2, xxx3;
    xxx0 = vec_mergeh(v0,v1);
    xxx1 = vec_mergel(v0,v1);
    xxx2 = vec_mergeh(v2,v3);
    xxx3 = vec_mergel(v2,v3);
    vsx_stxsdx(xxx0,x+ovs);
    vsx_stxsdx(xxx1,x+3*ovs);
    vsx_stxsdx(xxx2,x+ovs+2);
    vsx_stxsdx(xxx3,x+3*ovs+2);
    vsx_stxsdx(vsx_permdi(xxx0,xxx0,3),x);
    vsx_stxsdx(vsx_permdi(xxx1,xxx1,3),x+2*ovs);
    vsx_stxsdx(vsx_permdi(xxx2,xxx2,3),x+2);
    vsx_stxsdx(vsx_permdi(xxx3,xxx3,3),x+2*ovs+2);
}
#else /* !FFTW_SINGLE */

static inline void STM4(R *x, V v, INT ovs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     vsx_stxsdx(vsx_permdi(v,v,3),x);
     vsx_stxsdx(v,x+ovs);
}
#  define STN4(x, v0, v1, v2, v3, ovs) /* nothing */
#endif

static inline V VBYI(V x)
{
#ifdef FFTW_SINGLE
  const vector unsigned char perm = { 0xbb, 0xaa, 0x99, 0x88, 0xff, 0xee, 0xdd, 0xcc, 0x33, 0x22, 0x11, 0x00, 0x77, 0x66, 0x55, 0x44 };
#else
  const vector unsigned char perm = { 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00, 0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88 };
#endif
  const V                    pmpm = vec_mergel(vec_splats((R)0.0),vsx_neg(vec_splats((R)0.0)));
  /* We borrow a neat raid instruction that does a permute and xor in a single step */
  return vsx_vpermxor(x,pmpm,perm);
}

/* FMA support */
#define VFMA(a, b, c)  vec_madd(a,b,c)
#define VFNMS(a, b, c) vec_nmsub(a,b,c)
#define VFMS(a, b, c)  vec_msub(a,b,c)
#define VFMAI(b, c)    VADD(c, VBYI(b))
#define VFNMSI(b, c)   VSUB(c, VBYI(b))
#define VFMACONJ(b,c)  VADD(VCONJ(b),c)
#define VFMSCONJ(b,c)  VSUB(VCONJ(b),c)
#define VFNMSCONJ(b,c) VSUB(c, VCONJ(b))

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
#  define VTW1(v,x)  \
  {TW_COS, v, x}, {TW_COS, v+1, x}, {TW_SIN, v, x}, {TW_SIN, v+1, x}
static inline V BYTW1(const R *t, V sr)
{
     V tx = LDA(t,0,t);
     V tr = UNPCKH(tx, tx);
     V ti = UNPCKL(tx, tx);
     tr = VMUL(tr, sr);
     sr = VBYI(sr);
     return VFMA(ti, sr, tr);
}
static inline V BYTWJ1(const R *t, V sr)
{
     V tx = LDA(t,0,t);
     V tr = UNPCKH(tx, tx);
     V ti = UNPCKL(tx, tx);
     tr = VMUL(tr, sr);
     sr = VBYI(sr);
     return VFNMS(ti, sr, tr);
}
#else /* !FFTW_SINGLE */
#  define VTW1(v,x) {TW_CEXP, v, x}
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
#  define VTW2(v,x)							\
  {TW_COS, v, x}, {TW_COS, v, x}, {TW_COS, v+1, x}, {TW_COS, v+1, x},	\
  {TW_SIN, v, -x}, {TW_SIN, v, x}, {TW_SIN, v+1, -x}, {TW_SIN, v+1, x}
#else /* !FFTW_SINGLE */
#  define VTW2(v,x)							\
  {TW_COS, v, x}, {TW_COS, v, x}, {TW_SIN, v, -x}, {TW_SIN, v, x}
#endif
#define TWVL2 (2 * VL)
static inline V BYTW2(const R *t, V sr)
{
     V si = FLIP_RI(sr);
     V ti = LDA(t+2*VL,0,t);
     V tt = VMUL(ti, si);
     V tr = LDA(t,0,t);
     return VFMA(tr, sr, tt);
}
static inline V BYTWJ2(const R *t, V sr)
{
     V si = FLIP_RI(sr);
     V tr = LDA(t,0,t);
     V tt = VMUL(tr, sr);
     V ti = LDA(t+2*VL,0,t);
     return VFNMS(ti, si, tt);
}

/* twiddle storage #3 */
#ifdef FFTW_SINGLE
#  define VTW3(v,x) {TW_CEXP, v, x}, {TW_CEXP, v+1, x}
#  define TWVL3 (VL)
#else
#  define VTW3(v,x) VTW1(v,x)
#  define TWVL3 TWVL1
#endif

/* twiddle storage for split arrays */
#ifdef FFTW_SINGLE
#  define VTWS(v,x)							  \
    {TW_COS, v, x}, {TW_COS, v+1, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, \
    {TW_SIN, v, x}, {TW_SIN, v+1, x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, x}
#else
#  define VTWS(v,x)							  \
    {TW_COS, v, x}, {TW_COS, v+1, x}, {TW_SIN, v, x}, {TW_SIN, v+1, x}
#endif
#define TWVLS (2 * VL)

#define VLEAVE() /* nothing */

#include "simd-common.h"
