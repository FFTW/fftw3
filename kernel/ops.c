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

/* $Id: ops.c,v 1.1 2002-06-11 14:35:52 athena Exp $ */

#include "ifftw.h"

opcnt X(ops_add)(opcnt a, opcnt b)
{
     a.add += b.add;
     a.mul += b.mul;
     a.fma += b.fma;
     a.other += b.other;
     return a;
}

opcnt X(ops_add3)(opcnt a, opcnt b, opcnt c)
{
     return X(ops_add)(a, X(ops_add)(b, c));
}

opcnt X(ops_mul)(uint a, opcnt b)
{
     b.add *= a;
     b.mul *= a;
     b.fma *= a;
     b.other *= a;
     return b;
}

opcnt X(ops_other)(uint o)
{
     opcnt x;
     x.add = x.mul = x.fma = 0;
     x.other = o;
     return x;
}

const opcnt X(ops_zero) = {
     0, 0, 0, 0
};
