/*
 * Copyright (c) 2001 Matteo Frigo
 * Copyright (c) 2001 Steven G. Johnson
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

/* $Id: problem.c,v 1.3 2003-01-17 13:50:42 athena Exp $ */

#include "config.h"
#include "bench.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* do what I mean */
static void dwim(tensor *t)
{
     int i;
     iodim *d;
     if (!FINITE_RNK(t->rnk) || t->rnk < 1)
	  return;

     i = t->rnk;

     /* default last dimension strides */
     --i; d = t->dims + i;
     if (!d->is)
	  d->is = 1; 
     if (!d->os)
	  d->os = 1; 

     while (--i >= 0) {
	  d = t->dims + i;
	  if (!d[0].is) 
	       d[0].is = d[1].is * d[0].n; 
	  if (!d[0].os)
	       d[0].os = d[1].os * d[0].n; 
     }
}

static const char *parseint(const char *s, int *n)
{
     int sign = 1;

     *n = 0;

     if (*s == '-') { 
	  sign = -1;
	  ++s;
     } else if (*s == '+') { 
	  sign = +1; 
	  ++s; 
     }

     BENCH_ASSERT(isdigit(*s));
     while (isdigit(*s)) {
	  *n = *n * 10 + (*s - '0');
	  ++s;
     }
     
     *n *= sign;
     return s;
}

struct dimlist { iodim car; struct dimlist *cdr; };

static const char *parsetensor(const char *s, tensor **tp)
{
     struct dimlist *l = 0, *m;
     tensor *t;
     int rnk = 0;

 L1:
     m = (struct dimlist *)bench_malloc(sizeof(struct dimlist));
     /* nconc onto l */
     m->cdr = l; l = m;
     ++rnk; 

     s = parseint(s, &m->car.n);

     if (*s == ':') {
	  /* read input stride */
	  ++s;
	  s = parseint(s, &m->car.is);
	  if (*s == ':') {
	       /* read output stride */
	       ++s;
	       s = parseint(s, &m->car.os);
	  } else {
	       /* default */
	       m->car.os = m->car.is;
	  }
     } else {
	  m->car.is = 0;
	  m->car.os = 0;
     }

     if (*s == 'x' || *s == 'X' || *s == '*' || *s == ',') {
	  ++s;
	  goto L1;
     }
     
     /* now we have a dimlist.  Build tensor */
     t = mktensor(rnk);
     while (--rnk >= 0) {
	  iodim *d = t->dims + rnk;
	  BENCH_ASSERT(l);
	  m = l; l = m->cdr;
	  d->n = m->car.n;
	  d->is = m->car.is;
	  d->os = m->car.os;
	  bench_free(m);
     }

     dwim(t);

     *tp = tensor_compress(t);
     tensor_destroy(t);
     return s;
}

/* parse a problem description, return a problem */
struct problem *problem_parse(const char *s)
{
     struct problem *p;

     p = bench_malloc(sizeof(struct problem));

     p->kind = PROBLEM_COMPLEX;
     p->sign = -1;
     p->in = p->out = 0;
     p->inphys = p->outphys = 0;
     p->in_place = 0;
     p->split = 0;
     p->userinfo = 0;
     p->sz = p->vecsz = 0;

 L1:
     switch (tolower(*s)) {
	 case 'i': p->in_place = 1; ++s; goto L1;
	 case 'o': p->in_place = 0; ++s; goto L1;
	 case '/': p->split = 1; ++s; goto L1;
	 case 'f': 
	 case '-': p->sign = -1; ++s; goto L1;
	 case 'b': 
	 case '+': p->sign = 1; ++s; goto L1;
	 case 'r': p->kind = PROBLEM_REAL; ++s; goto L1;
	 case 'c': p->kind = PROBLEM_COMPLEX; ++s; goto L1;
	 default : ;
     }

     s = parsetensor(s, &p->sz);

     if (*s == '/' || *s == 'v' || *s == 'V') {
	  ++s;
	  s = parsetensor(s, &p->vecsz);
     } else {
	  p->vecsz = mktensor(0);
     }

     BENCH_ASSERT(p->sz && p->vecsz);
     return p;
}

void problem_destroy(struct problem *p)
{
     BENCH_ASSERT(p);
     problem_free(p);
     bench_free(p);
}

