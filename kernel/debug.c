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

/* $Id: debug.c,v 1.1 2002-06-18 14:33:58 athena Exp $ */
#include "ifftw.h"
#include <stdio.h>

static void putchr(printer *p, char c)
{
     UNUSED(p);
     putc(c, stderr);
}

void X(fftw_debug)(const char *format, ...)
{
     va_list ap;
     printer *p = X(mkprinter)(sizeof(printer), putchr);
     va_start(ap, format);
     p->vprint(p, format, ap);
     va_end(ap);
     X(printer_destroy)(p);
}
