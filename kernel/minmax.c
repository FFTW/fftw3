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

/* $Id: minmax.c,v 1.2 2002-06-07 22:07:53 athena Exp $ */

#include "ifftw.h"

int fftw_imax(int a, int b)
{
     return (a > b) ? a : b;
}

int fftw_imin(int a, int b)
{
     return (a < b) ? a : b;
}

uint fftw_uimax(uint a, uint b)
{
     return (a > b) ? a : b;
}

uint fftw_uimin(uint a, uint b)
{
     return (a < b) ? a : b;
}
