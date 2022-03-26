/*
 * Copyright (c) 2003, 2007-11 Matteo Frigo
 * Copyright (c) 2003, 2007-11 Massachusetts Institute of Technology
 *
 * RISC-V V support implemented by Romain Dolbeau. (c) 2019 Romain Dolbeau
 * Modified to support RVV spec v1.0 by Zheng Shuo. (c) 2022 Zheng Shuo
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
#  define TYPE(name) name ## _f32m1
#  define TYPEUINT(name) name ## _u32m1
#  define TYPEINTERPRETF2U(name) name ## _f32m1_u32m1
#  define TYPEINTERPRETU2F(name) name ## _u32m1_f32m1
#  define TYPEMEM(name) name ## e32_v_f32m1
#else
#  define DS(d,s) d /* double-precision option */
#  define TYPE(name) name ## _f64m1
#  define TYPEUINT(name) name ## _u64m1
#  define TYPEINTERPRETF2U(name) name ## _f64m1_u64m1
#  define TYPEINTERPRETU2F(name) name ## _u64m1_f64m1
#  define TYPEMEM(name) name ## e64_v_f64m1
#endif

// vlen up to 1024 to support qemu 7.0.0
#if RVV_VLEN == 1024
#  define VL DS(8, 16)        /* SIMD complex vector length */
#elif RVV_VLEN == 512
#  define VL DS(4, 8)        /* SIMD complex vector length */
#elif RVV_VLEN == 256
#  define VL DS(2, 4)        /* SIMD complex vector length */
#elif RVV_VLEN == 128
#  define VL DS(1, 2)        /* SIMD complex vector length */
#else
#  error "RVV_VLEN must be a power of 2 between 128 and 1024 bits temporarily"
#endif /* RVV_VLEN */

#define SIMD_VSTRIDE_OKA(x) ((x) == 2) 
#define SIMD_STRIDE_OKPAIR SIMD_STRIDE_OK

#define ZERO DS(0.0, 0.0f)

#include <riscv_vector.h>

typedef DS(vfloat64m1_t, vfloat32m1_t) V;
typedef DS(vuint64m1_t, vuint32m1_t) Vuint;

#define VNEG(a)   TYPE(vfneg_v)(a, 2*VL)
#define VADD(a, b) TYPE(vfadd_vv)(a, b, 2*VL)
#define VSUB(a, b) TYPE(vfsub_vv)(a, b, 2*VL)
#define VMUL(a, b) TYPE(vfmul_vv)(a, b, 2*VL)

// generate (all 1, 0, all 1, 0, ...) to split real and imagine parts
#define VPARTSPLIT TYPEUINT(vsub_vx)(TYPEUINT(vand_vx)(TYPEUINT(vid_v)(2*VL), 1, 2*VL), 1, 2*VL)

static inline V VDUPL(const V x)
{
	Vuint partr = VPARTSPLIT; // (all 1, 0, all 1, 0, ...)
	V xl = TYPEINTERPRETU2F(vreinterpret_v)(TYPEUINT(vand_vv)(TYPEINTERPRETF2U(vreinterpret_v)(x), partr, 2*VL)); // set odd elements to 0
	return VADD(TYPE(vfslide1up_vf)(xl, ZERO, 2*VL), xl);
}

static inline V VDUPH(const V x)
{
	Vuint partr = VPARTSPLIT; // (all 1, 0, all 1, 0, ...)
	Vuint parti = TYPEUINT(vnot_v)(partr, 2*VL); // (0, all 1, 0, all 1, ...)
	V xh = TYPEINTERPRETU2F(vreinterpret_v)(TYPEUINT(vand_vv)(TYPEINTERPRETF2U(vreinterpret_v)(x), parti, 2*VL)); // set even elements to 0
	return VADD(TYPE(vfslide1down_vf)(xh, ZERO, 2*VL), xh);
}

#define DVK(var, val) V var = TYPE(vfmv_v_f)(val, 2*VL)

static inline V FLIP_RI(const V x)
{
	Vuint partr = VPARTSPLIT; // (all 1, 0, all 1, 0, ...)
	V xl = TYPEINTERPRETU2F(vreinterpret_v)(TYPEUINT(vand_vv)(TYPEINTERPRETF2U(vreinterpret_v)(x), partr, 2*VL)); // set odd elements to 0
	Vuint parti = TYPEUINT(vnot_v)(partr, 2*VL); // (0, all 1, 0, all 1, ...)
	V xh = TYPEINTERPRETU2F(vreinterpret_v)(TYPEUINT(vand_vv)(TYPEINTERPRETF2U(vreinterpret_v)(x), parti, 2*VL)); // set even elements to 0
	return VADD(TYPE(vfslide1down_vf)(xh, ZERO, 2*VL), TYPE(vfslide1up_vf)(xl, ZERO, 2*VL));
}

static inline V VCONJ(const V x)
{
	Vuint partr = VPARTSPLIT; // (all 1, 0, all 1, 0, ...)
	V xl = TYPEINTERPRETU2F(vreinterpret_v)(TYPEUINT(vand_vv)(TYPEINTERPRETF2U(vreinterpret_v)(x), partr, 2*VL)); // set odd elements to 0
	Vuint parti = TYPEUINT(vnot_v)(partr, 2*VL); // (0, all 1, 0, all 1, ...)
	V xh = TYPEINTERPRETU2F(vreinterpret_v)(TYPEUINT(vand_vv)(TYPEINTERPRETF2U(vreinterpret_v)(x), parti, 2*VL)); // set even elements to 0
	return VADD(xl, VNEG(xh));
}

static inline V VBYI(V x)
{
	Vuint partr = VPARTSPLIT; // (all 1, 0, all 1, 0, ...)
	V xl = TYPEINTERPRETU2F(vreinterpret_v)(TYPEUINT(vand_vv)(TYPEINTERPRETF2U(vreinterpret_v)(x), partr, 2*VL)); // set odd elements to 0
	Vuint parti = TYPEUINT(vnot_v)(partr, 2*VL); // (0, all 1, 0, all 1, ...)
	V xh = TYPEINTERPRETU2F(vreinterpret_v)(TYPEUINT(vand_vv)(TYPEINTERPRETF2U(vreinterpret_v)(VNEG(x)), parti, 2*VL)); // set elements to negative, then set even elements to 0
	return VADD(TYPE(vfslide1down_vf)(xh, ZERO, 2*VL), TYPE(vfslide1up_vf)(xl, ZERO, 2*VL));
}

#define LDK(x) x

#define VFMA(a, b, c) TYPE(vfmacc_vv)(c, a, b, 2*VL)
#define VFMS(a, b, c) TYPE(vfmsac_vv)(c, a, b, 2*VL)
#define VFNMS(a, b, c) TYPE(vfnmsac_vv)(c, a, b, 2*VL)
#define VFMAI(b, c) VADD(c, VBYI(b))
#define VFNMSI(b, c) VSUB(c, VBYI(b))
#define VFMACONJ(b, c) VADD(VCONJ(b), c)
#define VFMSCONJ(b, c) VSUB(VCONJ(b), c)
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

static inline V LDA(const R *x, INT ivs, const R *aligned_like) {
    (void)aligned_like; /* UNUSED */

    return TYPEMEM(vl)(x, 2*VL);
}

static inline void STA(R *x, V v, INT ovs, const R *aligned_like) {
    (void)aligned_like; /* UNUSED */

    TYPEMEM(vs)(x, v, 2*VL);
}

static inline V LD(const R *x, INT ivs, const R *aligned_like)
{
	(void)aligned_like; /* UNUSED */

	V xl = TYPEMEM(vls)(x, sizeof(R)*ivs, VL);
	V xh = TYPEMEM(vls)(x+1, sizeof(R)*ivs, VL);

	Vuint idx = TYPEUINT(vid_v)(2*VL); // (0, 1, 2, 3, ...)
	Vuint idx1 = TYPEUINT(vand_vx)(idx, -2, 2*VL); // (0, 0, 2, 2, ...)
	Vuint idx2 = TYPEUINT(vsrl_vx)(idx1, 1, 2*VL); // (0, 0, 1, 1, ...)

	V xl1 = TYPE(vrgather_vv)(xl, idx2, 2*VL);
	V xh1 = TYPE(vrgather_vv)(xh, idx2, 2*VL);

	Vuint idx3 = TYPEUINT(vand_vx)(idx, 1, 2*VL); // (0, 1, 0, 1, ...)
	Vuint idx4 = TYPEUINT(vsub_vx)(idx3, 1, 2*VL); // (all 1, 0, all 1, 0, ...)
	Vuint idx5 = TYPEUINT(vnot_v)(idx4, 2*VL); // (0, all 1, 0, all 1, ...)

	V xl2 = TYPEINTERPRETU2F(vreinterpret_v)(TYPEUINT(vand_vv)(TYPEINTERPRETF2U(vreinterpret_v)(xl1), idx4, 2*VL)); // set odd elements to 0
	V xh2 = TYPEINTERPRETU2F(vreinterpret_v)(TYPEUINT(vand_vv)(TYPEINTERPRETF2U(vreinterpret_v)(xh1), idx5, 2*VL)); // set even elements to 0
	return VADD(xl2, xh2);
}

static inline void ST(R *x, V v, INT ovs, const R *aligned_like)
{
	(void)aligned_like; /* UNUSED */

	Vuint idx = TYPEUINT(vid_v)(VL); // (0, 1, 2, 3, ...)
	Vuint idx1 = TYPEUINT(vsll_vx)(idx, 1, VL); // (0, 2, 4, 6, ...)

	V vl = TYPE(vrgather_vv)(v, idx1, VL);
	TYPEMEM(vss)(x, sizeof(R)*ovs, vl, ovs ? VL : 1); // if ovs=0, store the first element

	Vuint idx2 = TYPEUINT(vadd_vx)(idx1, 1, VL); // (1, 3, 5, 7, ...)

	V vh = TYPE(vrgather_vv)(v, idx2, VL);
	TYPEMEM(vss)(x+1, sizeof(R)*ovs, vh, ovs ? VL : 1); // if ovs=0, store the first element
}

// only one of STM2 and STN2 should be implemented, according to the hardware. Both micros occur in the code, and the implemented one does some operations, while the no-op one is skipped.
#define STM2(x, v, ovs, a) ST(x, v, ovs, a)

#define STN2(x, v0, v1, ovs) /* no-op */

// only one of STM4 and STN4 should be implemented. See STM2 and STN2.
#define STM4(x, v, ovs, a) TYPEMEM(vss)(x, sizeof(R)*ovs, v, 2*VL)

#define STN4(x, v0, v1, v2, v3, ovs)  /* no-op */

/* twiddle storage #1: compact, slower */
#ifdef FFTW_SINGLE
#  if RVV_VLEN == 1024
#    define VTW1(v,x) {TW_CEXP, v, x}, {TW_CEXP, v+1, x}, {TW_CEXP, v+2, x}, {TW_CEXP, v+3, x}, \
		      {TW_CEXP, v+4, x}, {TW_CEXP, v+5, x}, {TW_CEXP, v+6, x}, {TW_CEXP, v+7, x}, \
		      {TW_CEXP, v+8, x}, {TW_CEXP, v+9, x}, {TW_CEXP, v+10, x}, {TW_CEXP, v+11, x}, \
		      {TW_CEXP, v+12, x}, {TW_CEXP, v+13, x}, {TW_CEXP, v+14, x}, {TW_CEXP, v+15, x}
#  elif RVV_VLEN == 512
#    define VTW1(v,x) {TW_CEXP, v, x}, {TW_CEXP, v+1, x}, {TW_CEXP, v+2, x}, {TW_CEXP, v+3, x}, \
		      {TW_CEXP, v+4, x}, {TW_CEXP, v+5, x}, {TW_CEXP, v+6, x}, {TW_CEXP, v+7, x}
#  elif RVV_VLEN == 256
#    define VTW1(v,x) {TW_CEXP, v, x}, {TW_CEXP, v+1, x}, {TW_CEXP, v+2, x}, {TW_CEXP, v+3, x}
#  elif RVV_VLEN == 128
#    define VTW1(v,x) {TW_CEXP, v, x}, {TW_CEXP, v+1, x}
#  else
#    error "RVV_VLEN must be a power of 2 between 128 and 1024 bits temporarily"
#  endif /* RVV_VLEN */
#else
#  if RVV_VLEN == 1024
#    define VTW1(v,x) {TW_CEXP, v, x}, {TW_CEXP, v+1, x}, {TW_CEXP, v+2, x}, {TW_CEXP, v+3, x}, \
		      {TW_CEXP, v+4, x}, {TW_CEXP, v+5, x}, {TW_CEXP, v+6, x}, {TW_CEXP, v+7, x}
#  elif RVV_VLEN == 512
#    define VTW1(v,x) {TW_CEXP, v, x}, {TW_CEXP, v+1, x}, {TW_CEXP, v+2, x}, {TW_CEXP, v+3, x}
#  elif RVV_VLEN == 256
#    define VTW1(v,x) {TW_CEXP, v, x}, {TW_CEXP, v+1, x}
#  elif RVV_VLEN == 128
#    define VTW1(v,x) {TW_CEXP, v, x}
#  else
#    error "RVV_VLEN must be a power of 2 between 128 and 1024 bits temporarily"
#  endif /* RVV_VLEN */
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
#  if RVV_VLEN == 1024
#    define VTW2(v,x) {TW_COS, v+0, x}, {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+1, x}, \
		      {TW_COS, v+2, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, {TW_COS, v+3, x}, \
		      {TW_COS, v+4, x}, {TW_COS, v+4, x}, {TW_COS, v+5, x}, {TW_COS, v+5, x}, \
		      {TW_COS, v+6, x}, {TW_COS, v+6, x}, {TW_COS, v+7, x}, {TW_COS, v+7, x}, \
		      {TW_COS, v+8, x}, {TW_COS, v+8, x}, {TW_COS, v+9, x}, {TW_COS, v+9, x}, \
		      {TW_COS, v+10, x}, {TW_COS, v+10, x}, {TW_COS, v+11, x}, {TW_COS, v+11, x}, \
		      {TW_COS, v+12, x}, {TW_COS, v+12, x}, {TW_COS, v+13, x}, {TW_COS, v+13, x}, \
		      {TW_COS, v+14, x}, {TW_COS, v+14, x}, {TW_COS, v+15, x}, {TW_COS, v+15, x}, \
		      {TW_SIN, v+0, -x}, {TW_SIN, v+0, x}, {TW_SIN, v+1, -x}, {TW_SIN, v+1, x}, \
		      {TW_SIN, v+2, -x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, -x}, {TW_SIN, v+3, x}, \
		      {TW_SIN, v+4, -x}, {TW_SIN, v+4, x}, {TW_SIN, v+5, -x}, {TW_SIN, v+5, x}, \
		      {TW_SIN, v+6, -x}, {TW_SIN, v+6, x}, {TW_SIN, v+7, -x}, {TW_SIN, v+7, x}, \
		      {TW_SIN, v+8, -x}, {TW_SIN, v+8, x}, {TW_SIN, v+9, -x}, {TW_SIN, v+9, x}, \
		      {TW_SIN, v+10, -x}, {TW_SIN, v+10, x}, {TW_SIN, v+11, -x}, {TW_SIN, v+11, x}, \
		      {TW_SIN, v+12, -x}, {TW_SIN, v+12, x}, {TW_SIN, v+13, -x}, {TW_SIN, v+13, x}, \
		      {TW_SIN, v+14, -x}, {TW_SIN, v+14, x}, {TW_SIN, v+15, -x}, {TW_SIN, v+15, x}
#  elif RVV_VLEN == 512
#    define VTW2(v,x) {TW_COS, v+0, x}, {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+1, x}, \
		      {TW_COS, v+2, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, {TW_COS, v+3, x}, \
		      {TW_COS, v+4, x}, {TW_COS, v+4, x}, {TW_COS, v+5, x}, {TW_COS, v+5, x}, \
		      {TW_COS, v+6, x}, {TW_COS, v+6, x}, {TW_COS, v+7, x}, {TW_COS, v+7, x}, \
		      {TW_SIN, v+0, -x}, {TW_SIN, v+0, x}, {TW_SIN, v+1, -x}, {TW_SIN, v+1, x}, \
		      {TW_SIN, v+2, -x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, -x}, {TW_SIN, v+3, x}, \
		      {TW_SIN, v+4, -x}, {TW_SIN, v+4, x}, {TW_SIN, v+5, -x}, {TW_SIN, v+5, x}, \
		      {TW_SIN, v+6, -x}, {TW_SIN, v+6, x}, {TW_SIN, v+7, -x}, {TW_SIN, v+7, x}
#  elif RVV_VLEN == 256
#    define VTW2(v,x) {TW_COS, v+0, x}, {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+1, x}, \
		      {TW_COS, v+2, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, {TW_COS, v+3, x}, \
		      {TW_SIN, v+0, -x}, {TW_SIN, v+0, x}, {TW_SIN, v+1, -x}, {TW_SIN, v+1, x}, \
		      {TW_SIN, v+2, -x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, -x}, {TW_SIN, v+3, x}
#  elif RVV_VLEN == 128
#    define VTW2(v,x) {TW_COS, v+0, x}, {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+1, x}, \
		      {TW_SIN, v+0, -x}, {TW_SIN, v+0, x}, {TW_SIN, v+1, -x}, {TW_SIN, v+1, x}
#  else
#    error "RVV_VLEN must be a power of 2 between 128 and 1024 bits temporarily"
#  endif /* RVV_VLEN */
#else
#  if RVV_VLEN == 1024
#    define VTW2(v,x) {TW_COS, v+0, x}, {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+1, x}, \
		      {TW_COS, v+2, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, {TW_COS, v+3, x}, \
		      {TW_COS, v+4, x}, {TW_COS, v+4, x}, {TW_COS, v+5, x}, {TW_COS, v+5, x}, \
		      {TW_COS, v+6, x}, {TW_COS, v+6, x}, {TW_COS, v+7, x}, {TW_COS, v+7, x}, \
		      {TW_SIN, v+0, -x}, {TW_SIN, v+0, x}, {TW_SIN, v+1, -x}, {TW_SIN, v+1, x}, \
		      {TW_SIN, v+2, -x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, -x}, {TW_SIN, v+3, x}, \
		      {TW_SIN, v+4, -x}, {TW_SIN, v+4, x}, {TW_SIN, v+5, -x}, {TW_SIN, v+5, x}, \
		      {TW_SIN, v+6, -x}, {TW_SIN, v+6, x}, {TW_SIN, v+7, -x}, {TW_SIN, v+7, x}
#  elif RVV_VLEN == 512
#    define VTW2(v,x) {TW_COS, v+0, x}, {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+1, x}, \
		      {TW_COS, v+2, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, {TW_COS, v+3, x}, \
		      {TW_SIN, v+0, -x}, {TW_SIN, v+0, x}, {TW_SIN, v+1, -x}, {TW_SIN, v+1, x}, \
		      {TW_SIN, v+2, -x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, -x}, {TW_SIN, v+3, x}
#  elif RVV_VLEN == 256
#    define VTW2(v,x) {TW_COS, v+0, x}, {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+1, x}, \
		      {TW_SIN, v+0, -x}, {TW_SIN, v+0, x}, {TW_SIN, v+1, -x}, {TW_SIN, v+1, x}
#  elif RVV_VLEN == 128
#    define VTW2(v,x) {TW_COS, v+0, x}, {TW_COS, v+0, x}, {TW_SIN, v+0, -x}, {TW_SIN, v+0, x}
#  else
#    error "RVV_VLEN must be a power of 2 between 128 and 1024 bits temporarily"
#  endif /* RVV_VLEN */
#endif

#define TWVL2 (2*VL)

static inline V BYTW2(const R *t, V sr)
{
    V si = FLIP_RI(sr);
    V ti = LDA(t+2*VL, 2, t+4*VL);
    V tr = LDA(t, 2, t);
    return VFMA(tr, sr, VMUL(ti, si));
}

static inline V BYTWJ2(const R *t, V sr)
{
    V si = FLIP_RI(sr);
    V ti = LDA(t+2*VL, 2, t+4*VL);
    V tr = LDA(t, 2, t);
    return VFNMS(ti, si, VMUL(tr, sr));
}

/* twiddle storage #3 */
#define VTW3(v,x) VTW1(v,x)
#define TWVL3 TWVL1

/* twiddle storage for split arrays */
#ifdef FFTW_SINGLE
#  if RVV_VLEN == 1024
#    define VTWS(v,x) {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, \
		      {TW_COS, v+4, x}, {TW_COS, v+5, x}, {TW_COS, v+6, x}, {TW_COS, v+7, x}, \
		      {TW_COS, v+8, x}, {TW_COS, v+9, x}, {TW_COS, v+10, x}, {TW_COS, v+11, x}, \
		      {TW_COS, v+12, x}, {TW_COS, v+13, x}, {TW_COS, v+14, x}, {TW_COS, v+15, x}, \
		      {TW_COS, v+16, x}, {TW_COS, v+17, x}, {TW_COS, v+18, x}, {TW_COS, v+19, x}, \
		      {TW_COS, v+20, x}, {TW_COS, v+21, x}, {TW_COS, v+22, x}, {TW_COS, v+23, x}, \
		      {TW_COS, v+24, x}, {TW_COS, v+25, x}, {TW_COS, v+26, x}, {TW_COS, v+27, x}, \
		      {TW_COS, v+28, x}, {TW_COS, v+29, x}, {TW_COS, v+30, x}, {TW_COS, v+31, x}, \
		      {TW_SIN, v+0, x}, {TW_SIN, v+1, x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, x}, \
		      {TW_SIN, v+4, x}, {TW_SIN, v+5, x}, {TW_SIN, v+6, x}, {TW_SIN, v+7, x}, \
		      {TW_SIN, v+8, x}, {TW_SIN, v+9, x}, {TW_SIN, v+10, x}, {TW_SIN, v+11, x}, \
		      {TW_SIN, v+12, x}, {TW_SIN, v+13, x}, {TW_SIN, v+14, x}, {TW_SIN, v+15, x}, \
		      {TW_SIN, v+16, x}, {TW_SIN, v+17, x}, {TW_SIN, v+18, x}, {TW_SIN, v+19, x}, \
		      {TW_SIN, v+20, x}, {TW_SIN, v+21, x}, {TW_SIN, v+22, x}, {TW_SIN, v+23, x}, \
		      {TW_SIN, v+24, x}, {TW_SIN, v+25, x}, {TW_SIN, v+26, x}, {TW_SIN, v+27, x}, \
		      {TW_SIN, v+28, x}, {TW_SIN, v+29, x}, {TW_SIN, v+30, x}, {TW_SIN, v+31, x}
#  elif RVV_VLEN == 512
#    define VTWS(v,x) {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, \
		      {TW_COS, v+4, x}, {TW_COS, v+5, x}, {TW_COS, v+6, x}, {TW_COS, v+7, x}, \
		      {TW_COS, v+8, x}, {TW_COS, v+9, x}, {TW_COS, v+10, x}, {TW_COS, v+11, x}, \
		      {TW_COS, v+12, x}, {TW_COS, v+13, x}, {TW_COS, v+14, x}, {TW_COS, v+15, x}, \
		      {TW_SIN, v+0, x}, {TW_SIN, v+1, x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, x}, \
		      {TW_SIN, v+4, x}, {TW_SIN, v+5, x}, {TW_SIN, v+6, x}, {TW_SIN, v+7, x}, \
		      {TW_SIN, v+8, x}, {TW_SIN, v+9, x}, {TW_SIN, v+10, x}, {TW_SIN, v+11, x}, \
		      {TW_SIN, v+12, x}, {TW_SIN, v+13, x}, {TW_SIN, v+14, x}, {TW_SIN, v+15, x}
#  elif RVV_VLEN == 256
#    define VTWS(v,x) {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, \
		      {TW_COS, v+4, x}, {TW_COS, v+5, x}, {TW_COS, v+6, x}, {TW_COS, v+7, x}, \
		      {TW_SIN, v+0, x}, {TW_SIN, v+1, x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, x}, \
		      {TW_SIN, v+4, x}, {TW_SIN, v+5, x}, {TW_SIN, v+6, x}, {TW_SIN, v+7, x}
#  elif RVV_VLEN == 128
#    define VTWS(v,x) {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, \
		      {TW_SIN, v+0, x}, {TW_SIN, v+1, x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, x}
#  else
#    error "RVV_VLEN must be a power of 2 between 128 and 1024 bits temporarily"
#  endif /* RVV_VLEN */
#else
#  if RVV_VLEN == 1024
#    define VTWS(v,x) {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, \
		      {TW_COS, v+4, x}, {TW_COS, v+5, x}, {TW_COS, v+6, x}, {TW_COS, v+7, x}, \
		      {TW_COS, v+8, x}, {TW_COS, v+9, x}, {TW_COS, v+10, x}, {TW_COS, v+11, x}, \
		      {TW_COS, v+12, x}, {TW_COS, v+13, x}, {TW_COS, v+14, x}, {TW_COS, v+15, x}, \
		      {TW_SIN, v+0, x}, {TW_SIN, v+1, x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, x}, \
		      {TW_SIN, v+4, x}, {TW_SIN, v+5, x}, {TW_SIN, v+6, x}, {TW_SIN, v+7, x}, \
		      {TW_SIN, v+8, x}, {TW_SIN, v+9, x}, {TW_SIN, v+10, x}, {TW_SIN, v+11, x}, \
		      {TW_SIN, v+12, x}, {TW_SIN, v+13, x}, {TW_SIN, v+14, x}, {TW_SIN, v+15, x}
#  elif RVV_VLEN == 512
#    define VTWS(v,x) {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, \
		      {TW_COS, v+4, x}, {TW_COS, v+5, x}, {TW_COS, v+6, x}, {TW_COS, v+7, x}, \
		      {TW_SIN, v+0, x}, {TW_SIN, v+1, x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, x}, \
		      {TW_SIN, v+4, x}, {TW_SIN, v+5, x}, {TW_SIN, v+6, x}, {TW_SIN, v+7, x}
#  elif RVV_VLEN == 256
#    define VTWS(v,x) {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_COS, v+2, x}, {TW_COS, v+3, x}, \
		      {TW_SIN, v+0, x}, {TW_SIN, v+1, x}, {TW_SIN, v+2, x}, {TW_SIN, v+3, x}
#  elif RVV_VLEN == 128
#    define VTWS(v,x) {TW_COS, v+0, x}, {TW_COS, v+1, x}, {TW_SIN, v+0, x}, {TW_SIN, v+1, x}
#  else
#    error "RVV_VLEN must be a power of 2 between 128 and 1024 bits temporarily"
#  endif /* RVV_VLEN */
#endif

#define TWVLS (2*VL)

#define VLEAVE() /* nothing */

#include "simd-common.h"
