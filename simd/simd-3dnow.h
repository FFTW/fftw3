/*
 * Copyright (c) 2003 Matteo Frigo
 * Copyright (c) 2003 Massachusetts Institute of Technology
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
#error "3DNow! only works in single precision"
#endif

#define VL 1            /* SIMD vector length, in term of complex numbers */
#define ALIGNMENT 8
#define ALIGNMENTA 8

typedef float V __attribute__ ((mode(V2SF),aligned(8)));

static __inline__ V VADD(V a, V b)
{
     V ret;
     __asm__("pfadd %2, %0" : "=y"(ret) : "%0"(a), "ym"(b));
     return ret;
}
static __inline__ V VSUB(V a, V b)
{
     V ret;
     __asm__("pfsub %2, %0" : "=y"(ret) : "0"(a), "ym"(b));
     return ret;
}
static __inline__ V VMUL(V b, V a)
{
     V ret;
     __asm__("pfmul %2, %0" : "=y"(ret) : "%0"(a), "ym"(b));
     return ret;
}

static __inline__ V PFACC(V b, V a)
{
     V ret;
     __asm__("pfacc %2, %0" : "=y"(ret) : "0"(a), "ym"(b));
     return ret;
}

static __inline__ V PFPNACC(V b, V a)
{
     V ret;
     __asm__("pfpnacc %2, %0" : "=y"(ret) : "0"(a), "ym"(b));
     return ret;
}

static __inline__ V VXOR(V b, V a)
{
     V ret;
     __asm__("pxor %2, %0" : "=y"(ret) : "%0"(a), "ym"(b));
     return ret;
}

#define DVK(var, val) V var = __extension__ ({		\
     static const union fvec _var = { {val, val} };	\
     _var.v;						\
})

static __inline__ V VSWAP(V a)
{
     V ret;
     __asm__("pswapd %1, %0" : "=y"(ret) : "ym"(a));
     return ret;
}

static __inline__ void BEGIN_SIMD(void)
{
     __asm__ __volatile__("femms");
}

static __inline__ void END_SIMD(void)
{
     __asm__ __volatile__("femms");
}

#define LDK(x) x

union fvec {
     float d[2];
     V v;
};

static __inline__ V LDA(const R *x, int ivs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     (void)ivs; /* UNUSED */
     return *(const V *)x;
}

static __inline__ void STA(R *x, V v, int ovs, const R *aligned_like)
{
     (void)aligned_like; /* UNUSED */
     (void)ovs; /* UNUSED */
     *(V *)x = v;
}

#define LD LDA
#define ST STA

static __inline__ V FLIP_RI(V x)
{
     return VSWAP(x);
}

extern const union fvec X(3dnow_mp);
static __inline__ V CHS_R(V x)
{
     return VXOR(X(3dnow_mp).v, x);
}

static __inline__ V VBYI(V x)
{
     return CHS_R(FLIP_RI(x));
}

#define VFMAI(b, c) VADD(c, VBYI(b))
#define VFNMSI(b, c) VSUB(c, VBYI(b))

static __inline__ V BYTW(const R *t, V ri)
{
     V tri = LD(t, 1, t);
     V ir = FLIP_RI(ri);
     V rrii = VMUL(tri, ri);
     V riir = VMUL(tri, ir);
     return PFPNACC(riir, rrii);
}

static __inline__ V BYTWJ(const R *t, V ri)
{
     V tri = LD(t, 1, t);
     V ir = FLIP_RI(ri);
     V rrii = VMUL(tri, ri);
     V riir = VMUL(tri, ir);
     return FLIP_RI(PFPNACC(rrii, riir));
}

#define VFMA(a, b, c) VADD(c, VMUL(a, b))
#define VFNMS(a, b, c) VSUB(c, VMUL(a, b))
#define VFMS(a, b, c) VSUB(VMUL(a, b), c)
#define VTW(x) {TW_COS, 0, x}, {TW_SIN, 0, x}
#define TWVL 1

#define RIGHT_CPU X(have_3dnow)
extern int RIGHT_CPU(void);

#define SIMD_VSTRIDE_OKA(x) 1
