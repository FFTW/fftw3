/*
 * Copyright (c) 2003, 2007-8 Matteo Frigo
 * Copyright (c) 2003, 2007-8 Massachusetts Institute of Technology
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

/* common routines for Cooley-Tukey algorithms */

#include "ifftw.h"

#define POW2P(n) (((n) > 0) && (((n) & ((n) - 1)) == 0))

/* TRUE if radix-r is ugly for size n */
int X(ct_uglyp)(INT min_n, INT v, INT n, INT r)
{
     return (n <= min_n) || (POW2P(n) && (v * (n / r)) <= 4);
}
