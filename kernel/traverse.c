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

/* $Id: traverse.c,v 1.4 2002-07-25 19:21:13 athena Exp $ */

#include "ifftw.h"
#include <stdarg.h>

typedef struct {
     printer super;
     visit_closure *k;
     int recur;
} traverser;

static void vtraverse(printer *p, const char *format, va_list ap)
{
     traverser *t = (traverser *) p;
     const char *s = format;
     char c;

     while ((c = *s++)) {
          if (c == '%') {
	       switch ((c = *s++)) {
		   case 'c':
			UNUSED(va_arg(ap, int));
			break;
		   case 's':
			UNUSED(va_arg(ap, char *));
			break;
		   case 'd':
			UNUSED(va_arg(ap, int));
			break;
		   case 'f': case 'e': case 'g':
			UNUSED(va_arg(ap, double));
			break;
		   case 'v':
			UNUSED(va_arg(ap, uint));
			break;
		   case 'o':
			UNUSED(va_arg(ap, int));
			break;
		   case 'u':
			UNUSED(va_arg(ap, uint));
			break;
		   case '(': case ')':
			break;
		   case 'p': {
			/* traverse plan */
			plan *x = va_arg(ap, plan *);
			if (x) {
			     if (t->recur) x->adt->print(x, p);
			     t->k->visit(t->k, x);
			}
			break;
		   }
		   case 'P':
			UNUSED(va_arg(ap, problem *));
			break;
		   case 't':
			UNUSED(va_arg(ap, tensor *));
			break;
		   default:
			A(0 /* unknown format */);
			break;
	       }
          }
     }
}

static void traverse(printer *p, const char *format, ...)
{
     va_list ap;
     va_start(ap, format);
     vtraverse(p, format, ap);
     va_end(ap);
}

static void putchr_nop(printer *p, char c) { UNUSED(p); UNUSED(c); }

void X(traverse_plan)(plan *p, int recur, visit_closure *k)
{
     traverser s;
     printer *pr = &s.super;

     pr->print = traverse;
     pr->vprint = vtraverse;
     pr->putchr = putchr_nop;
     pr->indent = pr->indent_incr = 0;
     s.k = k;
     s.recur = recur;

     p->adt->print(p, pr);
     k->visit(k, p);
}
