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

/* $Id: printers.c,v 1.1 2002-08-01 07:03:18 stevenj Exp $ */

#include "ifftw.h"

typedef struct {
     printer super;
     FILE *f;
} P_file;

static void putchr_file(printer *p_, char c)
{
     P_file *p = (P_file *) p_;
     fputc(c, p->f);
}

printer *X(mkprinter_file)(FILE *f)
{
     P_file *p = (P_file *) X(mkprinter)(sizeof(P_file), putchr_file);
     p->f = f;
     return &p->super;
}

typedef struct {
     printer super;
     uint *cnt;
} P_cnt;

static void putchr_cnt(printer *p_, char c)
{
     P_cnt *p = (P_cnt *) p_;
     UNUSED(c);
     ++*p->cnt;
}

printer *X(mkprinter_cnt)(uint *cnt)
{
     P_cnt *p = (P_cnt *) X(mkprinter)(sizeof(P_cnt), putchr_cnt);
     p->cnt = cnt;
     return &p->super;
}

typedef struct {
     printer super;
     char *s;
} P_str;

static void putchr_str(printer *p_, char c)
{
     P_str *p = (P_str *) p_;
     *p->s++ = c;
     *p->s = 0;
}

printer *X(mkprinter_str)(char *s)
{
     P_str *p = (P_str *) X(mkprinter)(sizeof(P_str), putchr_str);
     p->s = s;
     *s = 0;
     return &p->super;
}
