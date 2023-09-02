/*
 * Copyright (c) 2003, 2007-14 Matteo Frigo
 * Copyright (c) 2003, 2007-14 Massachusetts Institute of Technology
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
#  error "MSA only works in single/double precision"
#endif

#define SIMD_SUFFIX  _msa

#ifdef FFTW_SINGLE
#  define DS(d,s) s /* single-precision option */
#  define SUFF(name) name ## _w
#else
#  define DS(d,s) d /* double-precision option */
#  define SUFF(name) name ## _d
#endif

#define VL DS(1,2)         /* SIMD vector length, in term of complex numbers */
#define SIMD_VSTRIDE_OKA(x) DS(SIMD_STRIDE_OKA(x),((x) == 2))
#define SIMD_STRIDE_OKPAIR SIMD_STRIDE_OK

#if defined(__GNUC__) && !defined(FFTW_SINGLE) && !defined(__mips_msa)
#  error "compiling simd-msa.h in double precision without -mmsa"
#elif defined(__GNUC__) && defined(FFTW_SINGLE) && !defined(__mips_msa)
#  error "compiling simd-msa.h in single precision without -mmsa"
#endif

#include <msa.h>

typedef DS(v2f64, v4f32) V;
#define VADD SUFF(__builtin_msa_fadd)
#define VSUB SUFF(__builtin_msa_fsub)
#define VMUL SUFF(__builtin_msa_fmul)

#define LDK(x) x

static inline V VDUPL(V x)
{
#ifdef FFTW_SINGLE
    /* __builtin_shuffle(x, (v4i32){0, 0, 2, 2}); */
    return (V)__builtin_msa_shf_w((v4i32)x, 0xa0);
#else
    /* __builtin_shuffle(x, (v4i32){0, 1, 0, 1}) */
    return (V)__builtin_msa_shf_w((v4i32)x, 0x44);
#endif
}

static inline V VDUPH(V x)
{
#ifdef FFTW_SINGLE
    /* __builtin_shuffle(x, (v4i32){1, 1, 3, 3}); */
    return (V)__builtin_msa_shf_w((v4i32)x, 0xf5);
#else
    /* __builtin_shuffle(x, (v4i32){2, 3, 2, 3}) */
    return (V)__builtin_msa_shf_w((v4i32)x, 0xee);
#endif
}

#ifdef FFTW_SINGLE
static inline V MSA_FILL(float val)
{
    return (V)__builtin_msa_fill_w(*(int*)&val);
}
#else
static inline V MSA_FILL(double val)
{
    return (V)__builtin_msa_fill_d(*(long long int*)&val);
}
#endif

#define DVK(var, val) V var = MSA_FILL(val)

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
    (void)aligned_like; /* UNUSED */
    V resl = (V)__builtin_msa_ld_w(x, 0);
    V resh = (V)__builtin_msa_ld_w(x + ivs, 0);
    return (V)__builtin_msa_ilvr_d((v2i64)resh, (v2i64)resl);
}

static inline void ST(R *x, V v, INT ovs, const R *aligned_like)
{
    (void)aligned_like; /* UNUSED */
    *(x + ovs    ) = v[2];
    *(x + ovs + 1) = v[3];
    *(x    ) = v[0];
    *(x + 1) = v[1];
}

#else /* ! FFTW_SINGLE */
#  define LD LDA
#  define ST STA
#endif

#ifdef FFTW_SINGLE
#define STM2 ST
#define STN2(x, v0, v1, ovs) /* nop */

#define UNPCKL(a, b) (V)__builtin_msa_ilvr_w((v4i32)a, (v4i32)b)
#define UNPCKH(a, b) (V)__builtin_msa_ilvl_w((v4i32)a, (v4i32)b)

#  define STN4(x, v0, v1, v2, v3, ovs)          \
{                           \
     V xxx0, xxx1, xxx2, xxx3;              \
     xxx0 = UNPCKL(v0, v2);             \
     xxx1 = UNPCKH(v0, v2);             \
     xxx2 = UNPCKL(v1, v3);             \
     xxx3 = UNPCKH(v1, v3);             \
     STA(x, UNPCKL(xxx0, xxx2), 0, 0);          \
     STA(x + ovs, UNPCKH(xxx0, xxx2), 0, 0);        \
     STA(x + 2 * ovs, UNPCKL(xxx1, xxx3), 0, 0);    \
     STA(x + 3 * ovs, UNPCKH(xxx1, xxx3), 0, 0);    \
}

#define STM4(x, v, ovs, aligned_like) /* no-op */

#else
/* FFTW_DOUBLE */

#define STM2 STA
#define STN2(x, v0, v1, ovs) /* nop */

static inline void STM4(R *x, V v, INT ovs, const R *aligned_like)
{
    (void)aligned_like; /* UNUSED */
    *(x) = v[0];
    *(x+ovs) = v[1];
}
#  define STN4(x, v0, v1, v2, v3, ovs) /* nothing */
#endif

static inline V FLIP_RI(V x)
{
#ifdef FFTW_SINGLE
    /* __builtin_shuffle(x, (v4i32){1, 0, 3, 2}); */
    return (V)__builtin_msa_shf_w((v4i32)x, 0xb1);
#else
    /* __builtin_shuffle(x, (v4i32){2, 3, 0, 1}) */
    return (V)__builtin_msa_shf_w((v4i32)x, 0x4e);
#endif
}

static inline V VCONJ(V x)
{
#ifdef FFTW_SINGLE
    static const v16u8 pm = {0,0,0,0,0,0,0,0x80,0,0,0,0,0,0,0,0x80};
    return (V)__builtin_msa_xor_v((v16u8)x, pm);
#else
    static const v16u8 pm = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x80};
    return (V)__builtin_msa_xor_v((v16u8)x, pm);
#endif
}

static inline V VBYI(V x)
{
    return FLIP_RI(VCONJ(x));
}

#ifdef FFTW_SINGLE
#  define VLIT(x0, x1) {x0, x1, x0, x1}
#else
#  define VLIT(x0, x1) {x0, x1}
#endif

/* FMA support */
#ifdef MIPS_MSA_FMA
#  define VFMA(a, b, c) SUFF(__builtin_msa_fmadd)(a, b, c)
#  define VFNMS(a, b, c) SUFF(__builtin_msa_fmsub)(a, b, c)
#  define VFMS(a, b, c) (-SUFF(__builtin_msa_fmsub)(a, b, c))
#else
#  define VFMA(a, b, c) VADD(c, VMUL(a, b))
#  define VFNMS(a, b, c) VSUB(c, VMUL(a, b))
#  define VFMS(a, b, c) VSUB(VMUL(a, b), c)
#endif

static inline V VFMAI(V b, V c)
{
    static const V mp = VLIT(-1.0, 1.0);
    return VFMA(FLIP_RI(b), mp, c);
}

static inline V VFNMSI(V b, V c)
{
    static const V mp = VLIT(-1.0, 1.0);
    return VFNMS(FLIP_RI(b), mp, c);
}

static inline V VFMACONJ(V b, V c)
{
    static const V pm = VLIT(1.0, -1.0);
    return VFMA(b, pm, c);
}

static inline V VFNMSCONJ(V b, V c)
{
    static const V pm = VLIT(1.0, -1.0);
    return VFNMS(b, pm, c);
}

static inline V VFMSCONJ(V b, V c)
{
    return VSUB(VCONJ(b), c);
}

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
  {TW_CEXP, v, x}, {TW_CEXP, v+1, x}     
static inline V BYTW1(const R *t, V sr)
{
    return VZMUL(LDA(t, 2, t), sr);
}
static inline V BYTWJ1(const R *t, V sr)
{
    return VZMULJ(LDA(t, 2, t), sr);
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
#  define VTW2(v,x)                                                     \
  {TW_COS, v, x}, {TW_COS, v, x}, {TW_COS, v+1, x}, {TW_COS, v+1, x},   \
  {TW_SIN, v, -x}, {TW_SIN, v, x}, {TW_SIN, v+1, -x}, {TW_SIN, v+1, x}
#else /* !FFTW_SINGLE */
#  define VTW2(v,x)                                                     \
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
#  define VTW3(v,x) {TW_CEXP, v, x}, {TW_CEXP, v+1, x}
#  define TWVL3 (VL)
#else
#  define VTW3(v,x) VTW1(v,x)
#  define TWVL3 TWVL1
#endif

/* twiddle storage for split arrays */
#ifdef FFTW_SINGLE
#  define VTWS(v,x)                                                       \
    {TW_COS, v, x}, {TW_COS, v+1, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, \
    {TW_SIN, v, x}, {TW_SIN, v+1, x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, x}
#else
#  define VTWS(v,x)                                                       \
    {TW_COS, v, x}, {TW_COS, v+1, x}, {TW_SIN, v, x}, {TW_SIN, v+1, x}
#endif
#define TWVLS (2 * VL)

#define VLEAVE() /* nothing */

#include "simd-common.h"
