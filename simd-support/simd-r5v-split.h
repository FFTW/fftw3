/*
 * Copyright (c) 2003, 2007-11 Matteo Frigo
 * Copyright (c) 2003, 2007-11 Massachusetts Institute of Technology
 *
 * RISC-V V support implemented by Romain Dolbeau. (c) 2019 Romain Dolbeau
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#if defined(FFTW_LDOUBLE) || defined(FFTW_QUAD)
#error "RISC-V V vector instructions only works in single or double precision"
#endif

#ifdef FFTW_SINGLE
#  define DS(d,s) s /* single-precision option */
#  define TYPE(name) __riscv_ ## name ## _f32m1
#  define TYPE2(name) __riscv_ ## name ## _f32m1x2
#  define TYPEGET(name) __riscv_ ## name ## _f32m1x2_f32m1
#  define TYPESET(name) __riscv_ ## name ## _f32m1_f32m1x2
#  define TYPEINT(name) __riscv_ ## name ## _u32m1
#  define TYPEMASK(name) __riscv_ ## name ## _f32m1_m
#  define TYPEINTERPRETF2U(name) __riscv_ ## name ## _f32m1_u32m1
#  define TYPEINTERPRETU2F(name) __riscv_ ## name ## _u32m1_f32m1
#else /* !FFTW_SINGLE */
#  define DS(d,s) d /* double-precision option */
#  define TYPE(name) __riscv_ ## name ## _f64m1
#  define TYPE2(name) __riscv_ ## name ## _f64m1x2
#  define TYPEGET(name) __riscv_ ## name ## _f64m1x2_f64m1
#  define TYPESET(name) __riscv_ ## name ## _f64m1_f64m1x2
#  define TYPEINT(name) __riscv_ ## name ## _u64m1
#  define TYPEMASK(name) __riscv_ ## name ## _f64m1_m
#  define TYPEINTERPRETF2U(name) __riscv_ ## name ## _f64m1_u64m1
#  define TYPEINTERPRETU2F(name) __riscv_ ## name ## _u64m1_f64m1
#endif /* FFTW_SINGLE */

/* FIXME: this hardwire to 512 bits */
#if R5V_SIZE == 16384
#define VL DS(256, 512)        /* SIMD complex vector length */
#elif R5V_SIZE == 8192
#define VL DS(128, 256)        /* SIMD complex vector length */
#elif R5V_SIZE == 4096
#define VL DS(64, 128)        /* SIMD complex vector length */
#elif R5V_SIZE == 2048
#define VL DS(32, 64)        /* SIMD complex vector length */
#elif R5V_SIZE == 1024
#define VL DS(16, 32)        /* SIMD complex vector length */
#elif R5V_SIZE == 512
#define VL DS(8, 16)        /* SIMD complex vector length */
#elif R5V_SIZE == 256
#define VL DS(4, 8)        /* SIMD complex vector length */
#elif R5V_SIZE == 128
#define VL DS(2, 4)        /* SIMD complex vector length */
#else /* R5V_SIZE */
#error "R5V_SIZE must be a power of 2 between 128 and 16384 bits"
#endif /* R5V_SIZE */

#define SIMD_VSTRIDE_OKA(x) ((x) == 2) 
#define SIMD_STRIDE_OKPAIR SIMD_STRIDE_OK

#if defined(__GNUC__) && !defined(__riscv_vector) /* sanity check */
#error "compiling simd-r5v.h without RISC-V V support"
#endif

#include <riscv_vector.h>

typedef DS(vfloat64m1x2_t, vfloat32m1x2_t) V;
typedef DS(vfloat64m1_t, vfloat32m1_t) HALFV;
typedef DS(vuint64m1_t, vuint32m1_t) Vint;
typedef DS(vbool64_t, vbool32_t) Vmask;

/*
static inline V VLIT(const R re, const R im) {
  V v;
  v.v0 = TYPE(vfmv_v_f)(re, VL);
  v.v1 = TYPE(vfmv_v_f)(im, VL);
  return v;
}
*/
				 
static inline V VLIT1(const R val) {
  HALFV v0 = TYPE(vfmv_v_f)(val, VL);
  V v = TYPE2(vcreate_v)(v0, v0);
  return v;
}

#define LDK(x) x
#define DVK(var, val) V var = VLIT1(val)
//#define VZERO VLIT1(DS(0.,0.f))
//#define VRONE VLIT(DS(1.,1.f),DS(0.,0.f))
//#define VCI VLIT(DS(0.,0.f),DS(1.,1.f))
//#define VCONEMI VLIT(DS(1.,1.f),DS(-1.,-1.f))
//#define VONE  VLIT1(DS(1.,1.f))
//#define VMINUSONE VLIT1(DS(-1.,-1.f))

static inline V VDUPL(const V x) {
	HALFV v0 = TYPEGET(vget_v)(x, 0);
	//HALFV v1 = TYPEGET(vget_v)(x, 1);
	V v = TYPE2(vcreate_v)(v0, v0);
	return v;
}
static inline V VDUPH(const V x) {
	//HALFV v0 = TYPEGET(vget_v)(x, 0);
	HALFV v1 = TYPEGET(vget_v)(x, 1);
	V v = TYPE2(vcreate_v)(v1, v1);
	return v;
}

static inline V FLIP_RI(const V x) {
	HALFV v0 = TYPEGET(vget_v)(x, 0);
	HALFV v1 = TYPEGET(vget_v)(x, 1);
	V v = TYPE2(vcreate_v)(v1, v0);
	return v;
}

static inline V VCONJ(const V x) {
	HALFV v0 = TYPEGET(vget_v)(x, 0);
	HALFV v1 = TYPEGET(vget_v)(x, 1);
	V v = TYPE2(vcreate_v)(v0, TYPE(vfneg_v)(v1, VL));
	return v;
}

static inline V VBYI(const V x)
{
  V v;
  v = VCONJ(x);
  v = FLIP_RI(v);
  return v;
}

static inline V VNEG(const V x) {
	HALFV v0 = TYPEGET(vget_v)(x, 0);
	HALFV v1 = TYPEGET(vget_v)(x, 1);
	V v = TYPE2(vcreate_v)(TYPE(vfneg_v)(v0, VL), TYPE(vfneg_v)(v1, VL));
	return v;
}

#define BINOP(X,x)							\
  static inline V X(const V a, const V b) {				\
    HALFV av0 = TYPEGET(vget_v)(a, 0);					\
    HALFV av1 = TYPEGET(vget_v)(a, 1);					\
    HALFV bv0 = TYPEGET(vget_v)(b, 0);					\
    HALFV bv1 = TYPEGET(vget_v)(b, 1);					\
    V v = TYPE2(vcreate_v)(TYPE(x)(av0, bv0, VL), TYPE(x)(av1, bv1, VL)); \
    return v;								\
  }

BINOP(VADD,vfadd_vv)
BINOP(VSUB,vfsub_vv)
BINOP(VMUL,vfmul_vv)

#define TERNOP(X,x) \
  static inline V X(const V a, const V b, const V c) {			\
    HALFV av0 = TYPEGET(vget_v)(a, 0);					\
    HALFV av1 = TYPEGET(vget_v)(a, 1);					\
    HALFV bv0 = TYPEGET(vget_v)(b, 0);					\
    HALFV bv1 = TYPEGET(vget_v)(b, 1);					\
    HALFV cv0 = TYPEGET(vget_v)(c, 0);					\
    HALFV cv1 = TYPEGET(vget_v)(c, 1);					\
    V v = TYPE2(vcreate_v)(TYPE(x)(cv0, av0, bv0, VL), TYPE(x)(cv1, av1, bv1, VL)); \
    return v;								\
}

TERNOP(VFMA,vfmacc_vv) // or vfmadd
TERNOP(VFMS,vfmsac_vv)
TERNOP(VFNMS,vfnmsac_vv)

#define VFMAI(b, c) VADD(c, VBYI(b)) // fixme: improve
#define VFNMSI(b, c) VSUB(c, VBYI(b)) // fixme: improve
#define VFMACONJ(b,c)  VADD(VCONJ(b),c) // fixme: improve
#define VFMSCONJ(b,c)  VSUB(VCONJ(b),c) // fixme: improve
#define VFNMSCONJ(b,c) VSUB(c, VCONJ(b)) // fixme: improve

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

static inline V VZMULI(V tx, V sr) // fixme: improve
{
  V tr = VDUPL(tx);
  V ti = VDUPH(tx);
  ti = VMUL(ti, sr);
  sr = VBYI(sr);
  return VFMS(tr, sr, ti);
}

static inline V VZMULIJ(V tx, V sr) // fixme: improve
{
  V tr = VDUPL(tx);
  V ti = VDUPH(tx);
  ti = VMUL(ti, sr);
  sr = VBYI(sr);
  return VFMA(tr, sr, ti);
}

static inline V LDA(const R *x, INT ivs, const R *aligned_like) {
  (void)aligned_like; /* UNUSED */
  (void)ivs; /* UNUSED */
/* can be done with Zvlsseg instead */
#if 0
  HALFV v0 = TYPE(vlse64_v)(x, 2*sizeof(R), VL);
  HALFV v1 = TYPE(vlse64_v)(x+1, 2*sizeof(R), VL);
  V v = TYPE2(vcreate_v)(v0, v1);
#else
  V v;
#ifdef FFTW_SINGLE
  v = TYPE2(vlseg2e32_v)(x, VL);
#else !FFTW_SINGLE
  v = TYPE2(vlseg2e64_v)(x, VL);
#endif
#endif
  return v;
}
static inline void STA(R *x, const V v, INT ovs, const R *aligned_like) {
  (void)aligned_like; /* UNUSED */
  (void)ovs; /* UNUSED */
/* can be done with Zvlsseg instead */
#if 0
  HALFV v0 = TYPEGET(vget_v)(v, 0);
  HALFV v1 = TYPEGET(vget_v)(v, 1);
  TYPE(vsse64_v)(x, 2*sizeof(R), v0, VL);
  TYPE(vsse64_v)(x+1, 2*sizeof(R), v1, VL);
#else
#ifdef FFTW_SINGLE
  TYPE2(vsseg2e32_v)(x, v, VL);
#else !FFTW_SINGLE
  TYPE2(vsseg2e64_v)(x, v, VL);
#endif
#endif
}

#if FFTW_SINGLE

static inline V LDu(const R *x, INT ivs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */
  V v = TYPE2(vlsseg2e32_v)(x, ivs*sizeof(R), VL);
  return v;
}

static inline void STu(R *x, V v, INT ovs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */
  TYPE2(vssseg2e32_v)(x, ovs*sizeof(R), v, ovs? VL : 1);
}

#else /* !FFTW_SINGLE */

static inline V LDu(const R *x, INT ivs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */
#if 0
  HALFV v0 = TYPE(vlse64_v)(x, ivs*sizeof(R), VL);
  HALFV v1 = TYPE(vlse64_v)(x+1, ivs*sizeof(R), VL);
  V v = TYPE2(vcreate_v)(v0, v1);
#else
  V v = TYPE2(vlsseg2e64_v)(x, ivs*sizeof(R), VL);
#endif
  return v;
}

static inline void STu(R *x, V v, INT ovs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */
#if 0
  HALFV v0 = TYPEGET(vget_v)(v, 0);
  HALFV v1 = TYPEGET(vget_v)(v, 1);
  TYPE(vsse64_v)(x, ovs*sizeof(R), v0, VL); // fixme: check ovs == 0 ...
  TYPE(vsse64_v)(x+1, ovs*sizeof(R), v1, VL);
#else
  TYPE2(vssseg2e64_v)(x, ovs*sizeof(R), v, ovs? VL : 1);
#endif
}

#endif /* FFTW_SINGLE */

#define LD LDu
#define ST STu

#ifdef FFTW_SINGLE
#define STM2(x, v, ovs, a) ST(x, v, ovs, a)
#define STN2(x, v0, v1, ovs) /* nop */

static inline void STM4(R *x, V v, INT ovs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */
  HALFV v0 = TYPEGET(vget_v)(v, 0);
  HALFV v1 = TYPEGET(vget_v)(v, 1);
  TYPE(vsse32_v)(x,       2*ovs*sizeof(R), v0, VL); // fixme: check ovs == 0 ...
  TYPE(vsse32_v)(x + ovs, 2*ovs*sizeof(R), v1, VL);
}
#define STN4(x, v0, v1, v2, v3, ovs)  /* no-op */
#else /* !FFTW_SINGLE */
#define STM2(x, v, ovs, a) ST(x, v, ovs, a)
#define STN2(x, v0, v1, ovs) /* nop */

static inline void STM4(R *x, V v, INT ovs, const R *aligned_like)
{
  (void)aligned_like; /* UNUSED */
  HALFV v0 = TYPEGET(vget_v)(v, 0);
  HALFV v1 = TYPEGET(vget_v)(v, 1);
  TYPE(vsse64_v)(x,       2*ovs*sizeof(R), v0, VL); // fixme: check ovs == 0 ...
  TYPE(vsse64_v)(x + ovs, 2*ovs*sizeof(R), v1, VL);
  // 'segment' are dense so this wouldn't work
  // it would put (re,im) next to each other every ovs*sizeof(R)
  // it is STu
  //TYPE2(vssseg2e64_v)(x, ovs*sizeof(R), v, VL);
  // it could work with a vfloat64m2_t indexed store ?
  // TYPEM2(vsse64_v)(x, ovs*sizeof(R), v_as_m2, VL * 2);
  
}
// maybe would be better to do STN4 with two 4-wide segment store?
#define STN4(x, v0, v1, v2, v3, ovs)  /* no-op */
#endif /* FFTW_SINGLE */

/* twiddle storage #1: compact, slower */
#define REQ_VTW1
#define VTW_SIZE VL
#include "vtw.h"
#define TWVL1 (VL)
#undef VTW_SIZE
#undef REQ_VTW1

static inline V BYTW1(const R *t, V sr)
{
     return VZMUL(LDA(t, 2, t), sr);
}

static inline V BYTWJ1(const R *t, V sr)
{
     return VZMULJ(LDA(t, 2, t), sr);
}

/* twiddle storage #2: twice the space, faster (when in cache) */
#define REQ_VTW2
#define VTW_SIZE (2*VL)
#include "vtw.h"
#define TWVL2 (2*VL)
#undef VTW_SIZE
#undef REQ_VTW2

static inline V BYTW2(const R *t, V sr)
{
     V si = FLIP_RI(sr);
     V ti = LDA(t + 2*VL, 2, t + 4*VL);
     V tr = LDA(t, 2, t);
     return VFMA(tr, sr, VMUL(ti, si));
}

static inline V BYTWJ2(const R *t, V sr)
{
     V si = FLIP_RI(sr);
     V ti = LDA(t + 2*VL, 2, t + 4*VL);
     V tr = LDA(t, 2, t);
     return VFNMS(ti, si, VMUL(tr, sr));
}

/* twiddle storage #3 */
#define VTW3(v,x) VTW1(v,x)
#define TWVL3 TWVL1

/* twiddle storage for split arrays */
#define REQ_VTWS
#define VTW_SIZE (2*VL)
#include "vtw.h"
#define TWVLS (2*VL)
#undef VTW_SIZE
#undef REQ_VTWS

#define VLEAVE() /* nothing */

#include "simd-common.h"
