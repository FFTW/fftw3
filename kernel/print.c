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

/* $Id: print.c,v 1.3 2002-06-09 19:30:13 athena Exp $ */

#include "ifftw.h"
#include <stdarg.h>
#include <stdio.h>

#define BSZ 32

static void myputs(printer *p, const char *s)
{
     char c;
     while ((c = *s++))
	  p->putchr(p, c);
}

static void print(printer *p, const char *format, ...)
{
     char buf[BSZ];
     const char *s = format;
     char c;
     va_list ap;
     uint i;

     for (i = 0; i < p->indent; ++i)
	  p->putchr(p, ' ');

     va_start (ap, format);
     while ((c = *s++)) {
	  switch (c) {
	      case '%':
		   switch ((c = *s++)) {
		       case 's': {
			    char *x = va_arg(ap, char *);
			    myputs(p, x);
			    break;
		       }
		       case 'd': {
			    int x = va_arg(ap, int);
			    sprintf(buf, "%d", x);
			    goto putbuf;
		       }
		       case 'v': {
			    /* print optional vector length */
			    uint x = va_arg(ap, uint);
			    if (x > 0) {
				 sprintf(buf, "-x%u", x);
				 goto putbuf;
			    }
			    break;
		       }
		       case 'o': {
			    /* integer option.  Usage: %oNAME= */
			    int x = va_arg(ap, int);
			    if (x) p->putchr(p, '/');
			    while ((c = *s++) != '=')
				 if (x) p->putchr(p, c);
			    if (x) { 
				 sprintf(buf, "=%d", x);
				 goto putbuf;
			    }
			    break;
		       }
		       case 'u': {
			    uint x = va_arg(ap, uint);
			    sprintf(buf, "%u", x);
			    goto putbuf;
		       }
		       case 'p': {
			    /* print child plan */
			    plan *pln = va_arg(ap, plan *);
			    p->putchr(p, '\n');
			    p->indent += 2;
			    pln->adt->print(pln, p);
			    p->indent -= 2;
			    break;
		       }
		       default:
			    A(0 /* unknown format */);
			    break;

		       putbuf:
			    myputs(p, buf);
		   }
		   break;
	      default:
		   p->putchr(p, c);
		   break;
	  }
     }
     va_end(ap);
}

printer *fftw_mkprinter(size_t size, void (*putchr)(printer *p, char c))
{
     printer *s = (printer *)fftw_malloc(size, OTHER);
     s->print = print;
     s->putchr = putchr;
     s->indent = 0;
     return s;
}

void fftw_printer_destroy(printer *p)
{
     fftw_free(p);
}
