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

/* $Id: trig.c,v 1.2 2002-08-09 23:26:57 athena Exp $ */

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

/*
 * Improve accuracy by reducing x to range [0..1/8]
 * before multiplication by 2 * PI.
 */

trigreal sin2pi(long m, unsigned long n);
trigreal cos2pi(long m, unsigned long n)
{
     if (m < 0) return cos2pi(-m, n);
     if (2*m > n) return cos2pi(n-m, n);
     if (4*m > n) return -sin2pi(4*m - n, 4*n);
     if (8*m > n) return sin2pi(n - 4*m, 4*n);
     return COS(K2PI * ((trigreal)m/(trigreal)n));
}

trigreal sin2pi(long m, unsigned long n)
{
     if (m < 0) return -sin2pi(-m, n);
     if (2*m > n) return -sin2pi(n-m, n);
     if (4*m > n) return cos2pi(4*m - n, 4*n);
     if (8*m > n) return cos2pi(n - 4*m, 4*n);
     return SIN(K2PI * ((trigreal)m/(trigreal)n));
}

trigreal tan2pi(long m, unsigned long n)
{
     /* TODO: not implemented, unused */
     return TAN(K2PI * ((trigreal)m/(trigreal)n));
}
