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
#error "ALTIVEC only works in single precision"
#endif

#define VL 2            /* SIMD complex vector length */
#define ALIGNMENT 16

#ifndef __VEC__

/* simulate altivec spec by means of gcc-3.1 builtins */
#define vector __attribute__((vector_size(16)))
#define vec_add __builtin_altivec_vaddfp
#define vec_sub __builtin_altivec_vsubfp
#define vec_madd __builtin_altivec_vmaddfp
#define vec_nmsub __builtin_altivec_vnmsubfp
#define vec_lvsl(a1, a2) ((vector unsigned char) __builtin_altivec_lvsl ((a1), (a2)))

#define vec_lvsr(a1, a2) ((vector unsigned char) __builtin_altivec_lvsr ((a1), (a2)))

static inline vector float
vec_perm (vector float a1, vector float a2, vector unsigned char a3)
{
     return (vector float) __builtin_altivec_vperm_4si ((vector signed int) a1, (vector signed int) a2, (vector signed char) a3);
}

static inline vector float
vec_sel (vector float a1, vector float a2, vector unsigned int a3)
{
     return (vector float) __builtin_altivec_vsel_4si ((vector signed int) a1, (vector signed int) a2, (vector signed int) a3);
}

static inline vector float
vec_ld (int a1, float *a2)
{
     return (vector float) __builtin_altivec_lvx (a1, (void *) a2);
}

static inline void
vec_ste (vector float a1, int a2, void *a3)
{
     __builtin_altivec_stvewx ((vector signed int) a1, a2, (void *) a3);
}

static inline void
vec_st (vector float a1, int a2, void *a3)
{
     __builtin_altivec_stvx ((vector signed int) a1, a2, (void *) a3);
}

static inline vector float
vec_mergeh (vector float a1, vector float a2)
{
     return (vector float) __builtin_altivec_vmrghw ((vector signed int) a1, (vector signed int) a2);
}

static inline vector float
vec_mergel (vector float a1, vector float a2)
{
#if 1 /* gcc bug */
     vector float ret;
     __asm__("vmrglw %0, %1, %2" : "=v"(ret) : "v"(a1), "v"(a2));
     return ret;
#else
     return (vector float) __builtin_altivec_vmrglw ((vector signed int) a1, (vector signed int) a2);
#endif
}

#define VLIT4(x0, x1, x2, x3) {x0, x1, x2, x3}
#define VLIT16(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, xa, xb, xc, xd, xe, xf)\
 {x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, xa, xb, xc, xd, xe, xf}

#else /* !__VEC__ */

#define VLIT4(x0, x1, x2, x3) (x0, x1, x2, x3)
#define VLIT16(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, xa, xb, xc, xd, xe, xf)\
 (x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, xa, xb, xc, xd, xe, xf)

#endif

typedef vector float V;
#define VADD(a, b) vec_add(a, b)
#define VSUB(a, b) vec_sub(a, b)
#define VFMA(a, b, c) vec_madd(a, b, c)
#define VFNMS(a, b, c) vec_nmsub(a, b, c)
#define LDK(x) x
#define DVK(var, val) const V var = (vector float)VLIT4(val, val, val, val)

static inline V VMUL(V a, V b)
{
     DVK(zero, 0.0);
     return VFMA(a, b, zero);
}

static inline V VFMS(V a, V b, V c)
{
     return VSUB(VMUL(a, b), c);
}

/* load lower half */
static inline V LDL(const float *x, int ivs)
{
     int fivs = 4 * ivs;
     V v = vec_ld(fivs, (float *)x);
     return vec_perm(v, v, vec_lvsl(fivs, (float *)x));
}

static inline V LDH(const float *x)
{
     V v = vec_ld(0, (float *)x);
     return vec_perm(v, v, vec_lvsl(8, (float *)x));
}

static inline V LD(const float *x, int ivs)
{
     const vector unsigned int msk =
	  (vector unsigned int)VLIT4(0, 0, 0xFFFFFFFF, 0xFFFFFFFF);
     V l = LDL(x, ivs);
     V h = LDH(x);
     return vec_sel(l, h, msk);
}

/* store lower half */
static inline void STL(float *x, V v, int ovs)
{
     int fovs = 4 * ovs;
     v = vec_perm(v, v, vec_lvsr(fovs, x));
     vec_ste(v, fovs, x);
     vec_ste(v, 4 + fovs, x);
}

static inline void STH(float *x, V v)
{
     v = vec_perm(v, v, vec_lvsr(8, x));
     vec_ste(v, 0, x);
     vec_ste(v, 4, x);
}

static inline void ST(float *x, V v, int ovs)
{
     STL(x, v, ovs);
     STH(x, v);
}

static inline V FLIP_RI(V x)
{
     const vector unsigned char perm =
	  (vector unsigned char)VLIT16(4, 5, 6, 7, 0, 1, 2, 3,
				       12, 13, 14, 15, 8, 9, 10, 11);
     return vec_perm(x, x, perm);
}

static inline V CHS_R(V x)
{
     const vector float msk = (vector float)VLIT4(-1.0, 1.0, -1.0, 1.0);
     return VMUL(x, msk);
}

static inline V VBYI(V x)
{
     return CHS_R(FLIP_RI(x));
}

static inline V VFMAI(V b, V c)
{
     const vector float msk = (vector float)VLIT4(-1.0, 1.0, -1.0, 1.0);
     return VFMA(FLIP_RI(b), msk, c);
}

static inline V VFNMSI(V b, V c)
{
     const vector float msk = (vector float)VLIT4(-1.0, 1.0, -1.0, 1.0);
     return VFNMS(FLIP_RI(b), msk, c);
}

#define VTW(x) {TW_COS, 1, x}, {TW_COS, 0, x}, {TW_SIN, 1, x}, {TW_SIN, 0, x}
#define TWVL (VL)

static inline V BYTW(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V si = FLIP_RI(sr);
     V tx = twp[0];
     V tr = vec_mergeh(tx, tx);
     V ti = vec_mergel(tx, tx);
     return VFMA(CHS_R(ti), si, VMUL(tr, sr));
}

static inline V BYTWJ(const R *t, V sr)
{
     const V *twp = (const V *)t;
     V si = FLIP_RI(sr);
     V tx = twp[0];
     V tr = vec_mergeh(tx, tx);
     V ti = vec_mergel(tx, tx);
     return VFNMS(CHS_R(ti), si, VMUL(tr, sr));
}

#define RIGHT_CPU() 1

