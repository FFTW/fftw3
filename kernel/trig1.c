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

/* $Id: trig1.c,v 1.3 2003-03-15 20:29:43 stevenj Exp $ */

/* trigonometric functions */
#include "ifftw.h"
#include <math.h>

#ifdef FFTW_LDOUBLE
#  define COS cosl
#  define SIN sinl
#  define TAN tanl
#  define KTRIG(x) (x##L)
#else
#  define COS cos
#  define SIN sin
#  define TAN tan
#  define KTRIG(x) (x)
#endif

static const trigreal K2PI =
    KTRIG(6.2831853071795864769252867665590057683943388);
#define by2pi(m, n) ((K2PI * (m)) / (n))

/*
 * Improve accuracy by reducing x to range [0..1/8]
 * before multiplication by 2 * PI.
 */

trigreal X(sincos)(trigreal m, trigreal n, int sinp)
{
     /* waiting for C to get tail recursion... */
     trigreal half_n = n * KTRIG(0.5);
     trigreal quarter_n = half_n * KTRIG(0.5);
     trigreal eighth_n = quarter_n * KTRIG(0.5);
     trigreal sgn = KTRIG(1.0);

     if (sinp) goto sin;
 cos:
     if (m < 0) { m = -m; /* goto cos; */ }
     if (m > half_n) { m = n - m; goto cos; }
     if (m > eighth_n) { m = quarter_n - m; goto sin; }
     return sgn * COS(by2pi(m, n));

 msin:
     sgn = -sgn;
 sin:
     if (m < 0) { m = -m; goto msin; }
     if (m > half_n) { m = n - m; goto msin; }
     if (m > eighth_n) { m = quarter_n - m; goto cos; }
     return sgn * SIN(by2pi(m, n));
}
