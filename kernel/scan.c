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

/* $Id: scan.c,v 1.6 2002-08-31 01:29:10 athena Exp $ */

#include "ifftw.h"
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <ctype.h>

int X(scanner_getchr)(scanner *sc)
{
     if (sc->ungotc != EOF) {
	  int c = sc->ungotc;
	  sc->ungotc = EOF;
	  return c;
     }
     return(sc->getchr(sc));
}

#define GETCHR(sc) X(scanner_getchr)(sc)

void X(scanner_ungetchr)(scanner *sc, int c)
{
     sc->ungotc = c;
}

#define UNGETCHR(sc, c) X(scanner_ungetchr)(sc, c)

static void eat_blanks(scanner *sc)
{
     int ch;
     while (isspace(ch = GETCHR(sc)))
          ;
     UNGETCHR(sc, ch);
}

static void mygets(scanner *sc, char *s, size_t maxlen)
{
     char *s0 = s;
     int ch;

     A(maxlen > 0);
     eat_blanks(sc);
     while ((ch = GETCHR(sc)) != EOF && !isspace(ch)
	    && ch != ')' && ch != '(' && s < s0 + maxlen - 1)
	  *s++ = ch;
     *s = 0;
     UNGETCHR(sc, ch);
}

static long getlong(scanner *sc, int *ret)
{
     int sign = 1, ch;
     long x = 0;     

     eat_blanks(sc);
     ch = GETCHR(sc);
     if (ch == '-' || ch == '+') {
	  sign = ch == '-' ? -1 : 1;
	  ch = GETCHR(sc);
     }
     if (!isdigit(ch)) {
	  *ret = 0;
	  return x;
     }
     do {
	  x = x * 10 + ch - '0';
     } while (isdigit(ch = GETCHR(sc)));
     x *= sign;
     UNGETCHR(sc, ch);
     *ret = 1;
     return x;
}

#define BSZ 64
#define BUF(b, ch) {if (b - buf < BSZ - 1) *b++ = ch; else {*ret=0; return 0;}}

static R getR(scanner *sc, int *ret)
{
     char buf[BSZ], *b = buf;
     double x;
     int ch;

     eat_blanks(sc);
     /* read into buf: [+|-]ddd[.ddd][(e|E)[+|-]ddd] */
     ch = GETCHR(sc);
     if (ch == '+' || ch == '-') {
	  BUF(b, ch);
	  ch = GETCHR(sc);
     }
     while (isdigit(ch)) {
	  BUF(b, ch);
	  ch = GETCHR(sc);
     }
     if (ch == '.') {
	  BUF(b, ch);
	  while (isdigit(ch = GETCHR(sc)))
	       BUF(b, ch);
     }
     if (tolower(ch) == 'e') {
	  BUF(b, ch);
	  ch = GETCHR(sc);
	  if (ch == '+' || ch == '-') {
	       BUF(b, ch);
	       ch = GETCHR(sc);
	  }
	  while (isdigit(ch)) {
	       BUF(b, ch);
	       ch = GETCHR(sc);
	  }
     }
     UNGETCHR(sc, ch);
     *b = 0; /* terminate */
     *ret = sscanf(buf, "%lf", &x) == 1;
     return x;
}

static int getproblem(scanner *sc, problem **p)
{
     char buf[BSZ];

     *p = (problem *) 0;

     if (!sc->scan(sc, "(%*s", BSZ, buf)) return 0;

     if (!strcmp(buf, "null"))
	  return sc->scan(sc, ")");
     else {
	  const prbdesc *pp;
	  for (pp = sc->problems; pp && strcmp(pp->adt->nam,buf); pp = pp->cdr)
	       ;
	  if (!pp)
	       return 0;
	  if (!pp->adt->scan(sc, p)) return 0;
     }
     
     return sc->scan(sc, ")");
}

/* vscan is mostly scanf-like, with our additional format specifiers,
   but with a few twists.  It returns simply 0 or 1 indicating whether
   the match was successful. '(' and ')' in the format string match
   those characters preceded by any whitespace.  Finally, if a
   character match fails, it will ungetchr() the last character back
   onto the stream. */
static int vscan(scanner *sc, const char *format, va_list ap)
{
     const char *s = format;
     char c;
     int ch = 0;
     int fmt_len;

     while ((c = *s++)) {
	  fmt_len = 0;
          switch (c) {
	      case '%':
	  getformat:
		   switch ((c = *s++)) {
		       case 'c': {
			    char *x = va_arg(ap, char *);
			    ch = GETCHR(sc);
			    if (ch == EOF) return 0;
			    *x = ch;
			    break;
		       }
		       case 's': {
			    char *x = va_arg(ap, char *);
			    mygets(sc, x, fmt_len);
			    break;
		       }
		       case 'd': {
			    int *x = va_arg(ap, int *);
			    *x = (int) getlong(sc, &ch);
			    if (!ch) return 0;
			    break;
		       }
		       case 'u': {
			    uint *x = va_arg(ap, uint *);
			    *x = (uint) getlong(sc, &ch);
			    if (!ch) return 0;
			    break;
		       }
		       case 't': {
			    ptrdiff_t *x;
			    A(*s == 'd');
			    s += 1;
			    x = va_arg(ap, ptrdiff_t *);
			    *x = (ptrdiff_t) getlong(sc, &ch);
			    if (!ch) return 0;
			    break;
		       }
		       case 'f': case 'e': case 'g': {
			    R *x = va_arg(ap, R *);
			    *x = getR(sc, &ch);
			    if (!ch) return 0;
			    break;
		       }
		       case '(': case ')':
			    eat_blanks(sc);
			    break;
		       case 'P': {
			    /* scan problem */
			    problem **x = va_arg(ap, problem **);
			    if (!getproblem(sc, x)) {
				 if (*x) {
				      X(problem_destroy)(*x);
				      *x = 0;
				 }
				 return 0;
			    }
			    break;
		       }
		       case 'T': {
			    /* scan tensor */
			    tensor *x = va_arg(ap, tensor *);
			    if (!X(tensor_scan)(x, sc)) return 0;
			    break;
		       }
		       case '0': case '1': case '2': case '3': case '4':
		       case '5': case '6': case '7': case '8': case '9': {
			    fmt_len = c - '0';
			    while (isdigit(c = *s++))
				 fmt_len = fmt_len * 10 + c - '0';
			    UNGETCHR(sc, c);
			    goto getformat;
		       }
		       case '*': {
			    if ((fmt_len = va_arg(ap, int)) <= 0) return 0;
			    goto getformat;
		       }
		       default:
			    A(0 /* unknown format */);
			    break;
		   }
		   break;
	      default:
		   if (isspace(c) || c == '(' || c == ')')
			eat_blanks(sc);
		   if (!isspace(c) && (ch = GETCHR(sc)) != c) {
			UNGETCHR(sc, ch);
			return 0;
		   }
		   break;
          }
     }
     return 1;
}

static int scan(scanner *sc, const char *format, ...)
{
     int ret;
     va_list ap;
     va_start(ap, format);
     ret = vscan(sc, format, ap);
     va_end(ap);
     return ret;
}

scanner *X(mkscanner)(size_t size, int (*getchr)(scanner *sc),
		      const prbdesc *probs)
{
     scanner *s = (scanner *)fftw_malloc(size, OTHER);
     s->scan = scan;
     s->vscan = vscan;
     s->getchr = getchr;
     s->ungotc = EOF;
     s->problems = probs;
     return s;
}

void X(scanner_destroy)(scanner *sc)
{
     X(free)(sc);
}
