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

/* $Id: trig.c,v 1.1 2002-08-09 17:01:49 athena Exp $ */

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
 *
 * For n = 2^20, I tested sin2pi(m/n) and cos2pi(m/n)
 * for all m against the true value.  On Intel processors,
 * all results were correctly rounded (within 1/2ulp)
 * for both double and extended precision.
 *
 * Without the reduction, error was about 3ulp for both precisions.
 */
trigreal cos2pi(trigreal x)
{
     if (x < 0.0) return cos2pi(-x);
     if (x > 0.5) return cos2pi(1.0 - x);
     if (x > 0.25) return -sin2pi(x - 0.25);
     if (x > 0.125) return sin2pi(0.25 - x);
     return COS(K2PI * x);
}

trigreal sin2pi(trigreal x)
{
     if (x < 0.0) return -sin2pi(-x);
     if (x > 0.5) return -sin2pi(1.0 - x);
     if (x > 0.25) return cos2pi(x - 0.25);
     if (x > 0.125) return cos2pi(0.25 - x);
     return SIN(K2PI * x);
}

trigreal tan2pi(trigreal x)
{
     /* TODO: not implemented, unused */
     return TAN(K2PI * x);
}
