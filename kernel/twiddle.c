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

/* $Id: twiddle.c,v 1.23 2003-05-16 12:19:38 athena Exp $ */

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

static int ok_twid(const twid *t, const tw_instr *q, int n, int r, int m)
{
     return (n == t->n && r == t->r && m <= t->m && equal_instr(t->instr, q));
}

static twid *lookup(const tw_instr *q, int n, int r, int m)
{
     twid *p;

     for (p = twlist; p && !ok_twid(p, q, n, r, m); p = p->cdr)
          ;
     return p;
}

static int twlen0(int r, const tw_instr **pp)
{
     int ntwiddle = 0;
     const tw_instr *p = *pp;

     /* compute length of bytecode program */
     A(r > 0);
     for ( ; p->op != TW_NEXT; ++p) {
	  switch (p->op) {
	      case TW_FULL:
		   ntwiddle += (r - 1) * 2;
		   break;
	      case TW_HALF:
		   ntwiddle += (r - 1);
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

int X(twiddle_length)(int r, const tw_instr *p)
{
     return twlen0(r, &p);
}

static R *compute(const tw_instr *instr, int n, int r, int m)
{
     int ntwiddle, j;
     R *W, *W0;
     const tw_instr *p;

     static trigreal (*const f[])(int, int) = { 
	  X(cos2pi), X(sin2pi), X(tan2pi) 
     };

     p = instr;
     ntwiddle = twlen0(r, &p);

     W0 = W = (R *)MALLOC(ntwiddle * (m / p->v) * sizeof(R), TWIDDLES);

     for (j = 0; j < m; j += p->v) {
          for (p = instr; p->op != TW_NEXT; ++p) {
	       switch (p->op) {
		   case TW_FULL:
		   {
			int i;
			A((int)p->i == r); /* consistency check */
			A(m * r <= n);
			for (i = 1; i < r; ++i) {
			     *W++ = f[TW_COS]((j + p->v) * i, n);
			     *W++ = f[TW_SIN]((j + p->v) * i, n);
			}
			break;
		   }

		   case TW_HALF:
		   {
			int i;
			A((r % 2) == 1);
			for (i = 1; i + i < r; ++i) {
			     *W++ = f[TW_COS](MULMOD(i, (j + p->v), n), n);
			     *W++ = f[TW_SIN](MULMOD(i, (j + p->v), n), n);
			}
			break;
		   }

		   case TW_GENERIC:
		   {
			int i;
			A(p->v == 0); /* unused */
			A(p->i == 0); /* unused */
			A(m * r <= n);
			for (i = 0; i < r; ++i) {
			     int k = j * r + i;
			     *W++ = f[TW_COS](k, n);
			     *W++ = FFT_SIGN * f[TW_SIN](k, n);
			}
			break;
		   }
		   
		   default:
			A(m * r <= n);
			*W++ = f[p->op](((signed int)(j + p->v)) * p->i, n);
			break;
	       }
	  }
          A(m % p->v == 0);
     }

     return W0;
}

void X(mktwiddle)(twid **pp, const tw_instr *instr, int n, int r, int m)
{
     twid *p;

     if (*pp) return;  /* already created */

     if ((p = lookup(instr, n, r, m))) {
          ++p->refcnt;
	  goto done;
     }

     p = (twid *) MALLOC(sizeof(twid), TWIDDLES);
     p->n = n;
     p->r = r;
     p->m = m;
     p->instr = instr;
     p->refcnt = 1;
     p->W = compute(instr, n, r, m);

     /* cons! onto twlist */
     p->cdr = twlist;
     twlist = p;

 done:
     *pp = p;
     return;
}

void X(twiddle_destroy)(twid **pp)
{
     twid *p = *pp;
     if (p) {
          twid **q;
          if ((--p->refcnt) == 0) {
               /* remove p from twiddle list */
               for (q = &twlist; *q; q = &((*q)->cdr)) {
                    if (*q == p) {
                         *q = p->cdr;
                         X(ifree)(p->W);
                         X(ifree)(p);
			 goto done;
                    }
               }
               A(0 /* can't happen */ );
          }
     }
 done:
     *pp = 0; /* destroy pointer */
     return;
}


void X(twiddle_awake)(int flg, twid **pp, 
		      const tw_instr *instr, int n, int r, int m)
{
     if (flg) 
	  X(mktwiddle)(pp, instr, n, r, m);
     else 
	  X(twiddle_destroy)(pp);
}
