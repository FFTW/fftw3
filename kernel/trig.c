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

/* $Id: trig.c,v 1.3 2002-08-10 00:16:23 athena Exp $ */

/* trigonometric functions */
#include "ifftw.h"
#include <math.h>

#define sin2pi X(sin2pi)
#define cos2pi X(cos2pi)
#define tan2pi X(tan2pi)

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

/*
 * Improve accuracy by reducing x to range [0..1/8]
 * before multiplication by 2 * PI.
 */

static const trigreal KPIO2 =
    KTRIG(1.57079632679489661923132169163975144209858469968);

static trigreal sin2pi1(int m, int n, int k);
static trigreal cos2pi1(int m, int n, int k)
{
     if (m < 0) return cos2pi1(-m, n, -k);
     if (2 * m > n) return cos2pi1(n - m, n, -k);
     if (4 * m > n) return -sin2pi1(m - n / 4, n, k + 4 * (n / 4) - n);
     if (8 * m > n) return sin2pi1(n / 4 - m, n, -k + n - 4 * (n / 4));
     return COS(KPIO2 * (((trigreal) k + 4.0 * (trigreal)m) / (trigreal)n));
}

static trigreal sin2pi1(int m, int n, int k)
{
     if (m < 0) return -sin2pi1(-m, n, -k);
     if (2 * m > n) return -sin2pi1(n - m, n, -k);
     if (4 * m > n) return cos2pi1(m - n / 4, n, k + 4 * (n / 4) - n);
     if (8 * m > n) return cos2pi1(n / 4 - m, n, -k + n - 4 * (n / 4));
     return SIN(KPIO2 * (((trigreal) k + 4.0 * (trigreal)m) / (trigreal)n));
}

trigreal cos2pi(int m, int n)
{
     return cos2pi1(m, n, 0);
}

trigreal sin2pi(int m, int n)
{
     return sin2pi1(m, n, 0);
}
