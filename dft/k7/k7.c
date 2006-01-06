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

/* $Id: k7.c,v 1.9 2006-01-06 03:19:09 athena Exp $ */

#include "dft.h"

#if HAVE_K7

static inline int cpuid_edx(int op)
{
     int eax, ecx, edx;

     __asm__("push %%ebx\n\tcpuid\n\tpop %%ebx"
	     : "=a" (eax), "=c" (ecx), "=d" (edx)
	     : "a" (op));
     return edx;
}

static inline int cpuid_eax(int op)
{
     int eax, ecx, edx;

     __asm__("push %%ebx\n\tcpuid\n\tpop %%ebx"
	     : "=a" (eax), "=c" (ecx), "=d" (edx)
	     : "a" (op));
     return eax;
}

static int k7p(void)
{
     static int init = 0, res;

     if (!init) {
	  init = 1;
	  res = 0;

	  if (cpuid_eax(0x80000000) >= 0x80000001) 
	       res =  (cpuid_edx(0x80000001) >> 31)
		    & (cpuid_edx(0x80000001) >> 30) 
		    & 1;
     }
     return res;
}

/* cause a compile-time error if INT != int (in which case the
   assembly code dies horribly). */
static int nmokp(const kdft_desc *d,
		 const R *ri, const R *ii, const R *ro, const R *io,
		 INT is, INT os, INT vl, INT ivs, INT ovs,
		 const planner *plnr);

static int nmokp(const kdft_desc *d,
		 const R *ri, const R *ii, const R *ro, const R *io,
		 int is, int os, int vl, int ivs, int ovs,
		 const planner *plnr)
{
     return (k7p()
	     && !NO_SIMDP(plnr)
	     && ii == ri + 1 
	     && io == ro + 1
	     && (!d->is || (d->is == is))
	     && (!d->os || (d->is == os))
	  );
}

const kdft_genus X(kdft_k7_mgenus) = { nmokp, 1 };

static int npokp(const kdft_desc *d,
		 const R *ri, const R *ii, const R *ro, const R *io,
		 int is, int os, int vl, int ivs, int ovs,
		 const planner *plnr)
{
     return (k7p()
	     && !NO_SIMDP(plnr)
	     && ri == ii + 1 
	     && ro == io + 1
	     && (!d->is || (d->is == is))
	     && (!d->os || (d->is == os))
	  );
}

const kdft_genus X(kdft_k7_pgenus) = { npokp, 1 };

static int cmokp(const ct_desc *d,
		 const R *rio, const R *iio, 
		 int ios, int vs, int m, int mb, int me, int dist,
		 const planner *plnr)
{
     return (k7p()
	     && !NO_SIMDP(plnr)
	     && iio == rio + 1
	     && (!d->s1 || (d->s1 == ios))
	     && (!d->s2 || (d->s2 == vs))
	  );
}

const ct_genus X(kdft_ct_k7_mgenus) = { cmokp, 1 };

static int cpokp(const ct_desc *d,
		 const R *rio, const R *iio, 
		 int ios, int vs, int m, int mb, int me, int dist,
		 const planner *plnr)
{
     return (k7p()
	     && !NO_SIMDP(plnr)
	     && rio == iio + 1
	     && (!d->s1 || (d->s1 == ios))
	     && (!d->s2 || (d->s2 == vs))
	  );
}

const ct_genus X(kdft_ct_k7_pgenus) = { cpokp, 1 };
#endif
