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

/* $Id: align.c,v 1.9 2003-01-15 02:10:25 athena Exp $ */

#include "ifftw.h"
#include <stddef.h>

#if HAVE_SIMD
#include "simd.h"
#define ALGN SIMD_ALIGNMENT
#endif

#if HAVE_K7
#define ALGN 8
#endif

#ifndef ALGN
#define ALGN (sizeof(R))
#endif

int X(stride_aligned_p)(int s)
{
     return !(((unsigned)s * sizeof(R)) % ALGN);
}

/* NONPORTABLE */
int X(alignment_of)(R *p)
{
     return (int)(((unsigned long) p) % ALGN);
}
