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

/* $Id: traverse.c,v 1.2 2002-07-14 20:14:15 stevenj Exp $ */

#include "ifftw.h"
#include <stdarg.h>

typedef struct traverser_s traverser;

struct traverser_s {
     struct printer_s parent;
     void (*visit)(plan *p, void *closure);
     void *closure;
};

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
			     x->adt->print(x, p);
			     t->visit(x, t->closure);
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

void X(traverse_plan)(plan *p, void (*visit)(plan *, void *), void *closure)
{
     traverser *s = (traverser *)fftw_malloc(sizeof(traverser), OTHER);
     printer *pr = (printer *) s;

     s->parent.print = traverse;
     s->parent.vprint = vtraverse;
     s->parent.putchr = putchr_nop;
     s->parent.indent = s->parent.indent_incr = 0;
     s->visit = visit;
     s->closure = closure;

     p->adt->print(p, pr);
     visit(p, closure);
     X(free)(pr);
}
