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

/* $Id: trig.c,v 1.14 2003-03-15 20:29:43 stevenj Exp $ */

/* trigonometric functions */
#include "ifftw.h"
#include <math.h>

trigreal X(cos2pi)(int m, int n)
{
     return X(sincos)((trigreal)m, (trigreal)n, 0);
}

trigreal X(sin2pi)(int m, int n)
{
     return X(sincos)((trigreal)m, (trigreal)n, 1);
}

trigreal X(tan2pi)(int m, int n)
{
#if 0      /* unimplemented, unused */
     trigreal dm = m, dn = n;
     return TAN(by2pi(dm, dn));
#endif
     UNUSED(m); UNUSED(n);
     return 0.0;
}
