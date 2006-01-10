/*
 * Copyright (c) 2003, 2006 Matteo Frigo
 * Copyright (c) 2003, 2006 Massachusetts Institute of Technology
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

/* routines shared by the various buffered solvers */

#include "ifftw.h"

INT X(compute_nbuf)(INT n, INT vl, INT nbuf, INT maxbufsz)
{
     INT i; 

     if (nbuf * n > maxbufsz)
          nbuf = X(imax)((INT)1, maxbufsz / n);

     /*
      * Look for a buffer number (not too big) that divides the
      * vector length, in order that we only need one child plan:
      */
     for (i = nbuf; i < vl && i < 2 * nbuf; ++i)
          if (vl % i == 0)
               return i;

     /* whatever... */
     nbuf = X(imin)(nbuf, vl);
     return nbuf;
}

