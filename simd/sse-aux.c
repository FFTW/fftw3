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

/* $Id: sse-aux.c,v 1.3 2005-03-17 13:18:39 athena Exp $ */

#include "ifftw.h"
#include "simd.h"

#if HAVE_SSE

/* this declaration ought to be in sse.c, but icc-6.0
   misaligns the following vector.  The alignment is correct
   if we put the declaration in a separate file */

#if 0
/* apparently, MSVC converts -0.0 to 0.0 */
const union fvec X(sse_mpmp) = {{-0.0, 0.0, -0.0, 0.0}};
#endif

const union uvec X(sse_mpmp) = {
     { 0x80000000, 0x00000000, 0x80000000, 0x00000000 }
};
#endif
