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

/* $Id: twiddle.c,v 1.1 2002-06-05 00:03:56 athena Exp $ */

/* Twiddle manipulation */

#include "ifftw.h"
#include <math.h>

/* change here if you need higher precision */
typedef double trigreal;
#define COS(x) cos(x)
#define SIN(x) sin(x)
#define TAN(x) tan(x)
#define K2PI ((trigreal)6.2831853071795864769252867665590057683943388)

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

static int equal_twid(const twid *t, const tw_instr *q, uint r, int m)
{
     return (r == t->r && m == t->m && equal_instr(t->instr, q));
}

static twid *lookup(const tw_instr *q, uint r, int m)
{
     twid *p;

     for (p = twlist; p && !equal_twid(p, q, r, m); p = p->cdr) 
	  ;
     return p;
}

static R *compute(const tw_instr *instr, uint r, int m)
{
     int n = r * m;
     int ntwiddle;
     R *W, *W0;
     int v;
     const tw_instr *p;
     trigreal twoPiOverN = K2PI / (trigreal) n;

     /* compute length of bytecode program */
     for (ntwiddle = 0, p = instr; p->op != TW_NEXT; ++p)
	  ++ntwiddle;
     A(m % p->v == 0);

     W0 = W = (R *)fftw_malloc(ntwiddle * (m / p->v) * sizeof(R), TWIDDLES);

     for (v = 0; v < m; v += p->v) {
	  for (p = instr; p->op != TW_NEXT; ++p) {
	       trigreal ij = (trigreal) ((v + p->v) * p->i);
	       ij *= twoPiOverN;
	       switch (p->op) {
		   case TW_COS: *W++ = COS(ij); break;
		   case TW_SIN: *W++ = SIN(ij); break;
		   case TW_TAN: *W++ = TAN(ij); break;
		   default: A(0 /* can't happen */ );
	       }
	  }
     }

     return W0;
}

twid *fftw_mktwiddle(const tw_instr *instr, uint r, int m)
{
     twid *p;

     if ((p = lookup(instr, r, m))) {
	  ++p->refcnt;
	  return p;
     }

     p = (twid *) fftw_malloc(sizeof(twid), TWIDDLES);
     p->r = r;
     p->m = m;
     p->instr = instr;
     p->W = compute(instr, r, m);
     p->refcnt = 1;

     /* cons! onto twlist */
     p->cdr = twlist;
     twlist = p;

     return p;
}

void fftw_twiddle_destroy(twid *p)
{
     twid **q;
     if ((--p->refcnt) == 0) {
	  /* remove p from twiddle list */
	  for (q = &twlist; *q; q = &((*q)->cdr)) {
	       if (*q == p) {
		    *q = p->cdr;
		    fftw_free(p->W);
		    fftw_free(p);
		    return;
	       }
	  }

	  A(0 /* can't happen */ );
     }
}
