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

/* $Id: scanners.c,v 1.3 2002-09-01 23:51:50 athena Exp $ */

#include "ifftw.h"

typedef struct {
     scanner super;
     FILE *f;
} S_file;

static int getchr_file(scanner *sc_)
{
     S_file *sc = (S_file *) sc_;
     return fgetc(sc->f);
}

scanner *X(mkscanner_file)(FILE *f)
{
     S_file *sc = (S_file *) X(mkscanner)(sizeof(S_file), getchr_file);
     sc->f = f;
     return &sc->super;
}

typedef struct {
     scanner super;
     const char *s;
} S_str;

static int getchr_str(scanner *sc_)
{
     S_str *sc = (S_str *) sc_;
     if (!*sc->s)
	  return EOF;
     return *sc->s++;
}

scanner *X(mkscanner_str)(const char *s)
{
     S_str *sc = (S_str *) X(mkscanner)(sizeof(S_str), getchr_str);
     sc->s = s;
     return &sc->super;
}
