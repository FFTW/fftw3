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

/* $Id: twiddle.c,v 1.12 2002-07-27 19:06:21 athena Exp $ */

/* Twiddle manipulation */

#include "ifftw.h"
#include <math.h>

/* table of known twiddle factors */
static twid *twlist = (twid *) 0;

static int equal_instr(const tw_instr *p, const tw_instr *q)
{
     if (p == q)
          return 1;

     for (;; ++p, ++q) {
          if (p->op != q->op || p->v != q->v || p->i != q->i)
               return 0;
          if (p->op == TW_NEXT)  /* == q->op */
               return 1;
     }
     A(0 /* can't happen */);
}

static int ok_twid(const twid *t, const tw_instr *q, uint n, uint r, uint m)
{
     return (n == t->n && r == t->r && m <= t->m && equal_instr(t->instr, q));
}

static twid *lookup(const tw_instr *q, uint n, uint r, uint m)
{
     twid *p;

     for (p = twlist; p && !ok_twid(p, q, n, r, m); p = p->cdr)
          ;
     return p;
}

static uint twlen0(uint r, const tw_instr **pp)
{
     uint ntwiddle = 0;
     const tw_instr *p = *pp;

     /* compute length of bytecode program */
     A(r > 0);
     for ( ; p->op != TW_NEXT; ++p) {
	  switch (p->op) {
	      case TW_FULL:
		   ntwiddle += (r - 1) * 2;
		   break;
	      case TW_GENERIC:
		   ntwiddle += r * 2;
		   break;
	      default:
		   ++ntwiddle;
	  }
     }

     *pp = p;
     return ntwiddle;
}

uint X(twiddle_length)(uint r, const tw_instr *p)
{
     return twlen0(r, &p);
}

static R *compute(const tw_instr *instr, uint n, uint r, uint m)
{
     uint ntwiddle, j;
     R *W, *W0;
     const tw_instr *p;
     trigreal twoPiOverN = K2PI / (trigreal) n;

     static trigreal (*const f[])(trigreal) = { COS, SIN, TAN };

     p = instr;
     ntwiddle = twlen0(r, &p);

     W0 = W = (R *)fftw_malloc(ntwiddle * (m / p->v) * sizeof(R), TWIDDLES);

     for (j = 0; j < m; j += p->v) {
          for (p = instr; p->op != TW_NEXT; ++p) {
	       switch (p->op) {
		   case TW_FULL:
		   {
			uint i;
			A(p->i == r); /* consistency check */
			for (i = 1; i < r; ++i) {
			     *W++ = COS(twoPiOverN * ((j + p->v) * i));
			     *W++ = SIN(twoPiOverN * ((j + p->v) * i));
			}
			break;
		   }

		   case TW_GENERIC:
		   {
			uint i;
			A(p->v == 0); /* unused */
			A(p->i == 0); /* unused */
			for (i = 0; i < r; ++i) {
			     uint k = j * r + i;
			     *W++ = COS(twoPiOverN * k);
			     *W++ = FFT_SIGN * SIN(twoPiOverN * k);
			}
			break;
		   }
		   
		   default:
			*W++ = f[p->op](twoPiOverN * 
					(((signed int)(j + p->v)) * p->i));
			break;
	       }
	  }
          A(m % p->v == 0);
     }

     return W0;
}

twid *X(mktwiddle)(const tw_instr *instr, uint n, uint r, uint m)
{
     twid *p;

     if ((p = lookup(instr, n, r, m))) {
          ++p->refcnt;
          return p;
     }

     p = (twid *) fftw_malloc(sizeof(twid), TWIDDLES);
     p->n = n;
     p->r = r;
     p->m = m;
     p->instr = instr;
     p->refcnt = 1;
     p->W = compute(instr, n, r, m);

     /* cons! onto twlist */
     p->cdr = twlist;
     twlist = p;

     return p;
}

void X(twiddle_destroy)(twid *p)
{
     if (p) {
          twid **q;
          if ((--p->refcnt) == 0) {
               /* remove p from twiddle list */
               for (q = &twlist; *q; q = &((*q)->cdr)) {
                    if (*q == p) {
                         *q = p->cdr;
                         X(free)(p->W);
                         X(free)(p);
                         return;
                    }
               }

               A(0 /* can't happen */ );
          }
     }
}
