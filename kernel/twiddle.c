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

/* $Id: twiddle.c,v 1.3 2002-06-05 20:03:44 athena Exp $ */

/* Twiddle manipulation */

#include "ifftw.h"
#include <math.h>

/* change here if you need higher precision */
typedef double trigreal;
#define COS cos
#define SIN sin
#define TAN tan
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

static int equal_twid(const twid *t, const tw_instr *q, uint r, uint m)
{
     return (r == t->r && m == t->m && equal_instr(t->instr, q));
}

static twid *lookup(const tw_instr *q, uint r, uint m)
{
     twid *p;

     for (p = twlist; p && !equal_twid(p, q, r, m); p = p->cdr) 
	  ;
     return p;
}

uint fftw_twiddle_length(const tw_instr *p)
{
     uint ntwiddle = 0;

     /* compute length of bytecode program */
     for ( ; p->op != TW_NEXT; ++p)
	  ++ntwiddle;
     return ntwiddle;
}

static R *compute(const tw_instr *instr, uint r, uint m)
{
     uint ntwiddle, j, n = r * m;
     R *W, *W0;
     const tw_instr *p;
     trigreal twoPiOverN = K2PI / (trigreal) n;

     static trigreal (*const f[])(trigreal) = { COS, SIN, TAN };

     ntwiddle = fftw_twiddle_length(instr);

     W0 = W = (R *)fftw_malloc(ntwiddle * (m / p->v) * sizeof(R), TWIDDLES);

     for (j = 0; j < m; j += p->v) {
	  for (p = instr; p->op != TW_NEXT; ++p) 
	       *W++ = f[p->op](twoPiOverN * ((j + p->v) * p->i));
	  A(m % p->v == 0);
     }

     return W0;
}

twid *fftw_mktwiddle(const tw_instr *instr, uint r, uint m)
{
     twid *p;

     if ((p = lookup(instr, r, m))) {
	  ++p->refcnt;
	  return p;
     }

     p = (twid *) fftw_malloc(sizeof(twid), TWIDDLES);
     p->r = r;  p->m = m;  p->instr = instr;  p->refcnt = 1;
     p->W = compute(instr, r, m);

     /* cons! onto twlist */
     p->cdr = twlist; twlist = p;

     return p;
}

void fftw_twiddle_destroy(twid *p)
{
     if (p) {
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
}
