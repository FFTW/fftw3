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

/* $Id: trig.c,v 1.9 2002-08-20 12:37:45 athena Exp $ */

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

static const trigreal K2PI =
    KTRIG(6.2831853071795864769252867665590057683943388);
#define by2pi(m, n) ((K2PI * (m)) / (n))

/*
 * Improve accuracy by reducing x to range [0..1/8]
 * before multiplication by 2 * PI.
 */

static trigreal sin2pi0(trigreal m, trigreal n);
static trigreal cos2pi0(trigreal m, trigreal n)
{
     if (m < 0) return cos2pi0(-m, n);
     while (m >= n) m -= n;
     if (m > n * 0.5) return cos2pi0(n - m, n);
     if (m > n * 0.25) return -sin2pi0(m - n * 0.25, n);
     if (m > n * 0.125) return sin2pi0(n * 0.25 - m, n);
     return COS(by2pi(m, n));
}

static trigreal sin2pi0(trigreal m, trigreal n)
{
     if (m < 0) return -sin2pi0(-m, n);
     while (m >= n) m -= n;
     if (m > n * 0.5) return -sin2pi0(n - m, n);
     if (m > n * 0.25) return cos2pi0(m - n * 0.25, n);
     if (m > n * 0.125) return cos2pi0(n * 0.25 - m, n);
     return SIN(by2pi(m, n));
}

trigreal cos2pi(int m, uint n)
{
     return cos2pi0((trigreal)m, (trigreal)n);
}

trigreal sin2pi(int m, uint n)
{
     return sin2pi0((trigreal)m, (trigreal)n);
}

trigreal tan2pi(int m, uint n)
{
     trigreal dm = m, dn = n;
     /* unimplemented, unused */
     return TAN(by2pi(dm, dn));
}
