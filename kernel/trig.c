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

/* $Id: trig.c,v 1.5 2002-08-10 00:38:07 athena Exp $ */

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

static const trigreal K2PI =
    KTRIG(6.2831853071795864769252867665590057683943388);

static const uint MAXN = 1 << ((8 * sizeof(int)) - 3);

static trigreal sin2pi(int m, uint n);
static trigreal cos2pi(int m, uint n)
{
     if (m < 0) return cos2pi(-m, n);
     if (m > n / 2) return cos2pi(n - m, n);
     if (n % 4 == 0) {
	  if (m > n / 4) return -sin2pi(m - n / 4, n);
	  if (2 * m > n / 4) return sin2pi(n / 4 - m, n);
     } else if (n < MAXN) {
	  if (4 * m > n) return -sin2pi(4 * m - n, 4 * n);
	  if (8 * m > n) return sin2pi(n - 4 * m, 4 * n);
     }
     return COS(K2PI * ((trigreal)m / (trigreal)n));
}

static trigreal sin2pi(int m, uint n)
{
     if (m < 0) return -sin2pi(-m, n);
     if (m > n / 2) return -sin2pi(n - m, n);
     if (n % 4 == 0) {
	  if (m > n / 4) return cos2pi(m - n / 4, n);
	  if (2 * m > n / 4) return cos2pi(n / 4 - m, n);
     } else if (n < MAXN) {
	  if (4 * m > n) return cos2pi(4 * m - n, 4 * n);
	  if (8 * m > n) return cos2pi(n - 4 * m, 4 * n);
     }
     return SIN(K2PI * ((trigreal)m / (trigreal)n));
}

trigreal tan2pi(int m, uint n)
{
     /* unimplemented, unused */
     return TAN(K2PI * ((trigreal)m / (trigreal)n));
}
