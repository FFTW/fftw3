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

#include "api.h"

typedef struct {
     scanner super;
     FILE *f;
} S_file;

static int getchr_file(scanner * sc_)
{
     S_file *sc = (S_file *) sc_;
     return fgetc(sc->f);
}

static scanner *mkscanner_file(FILE *f)
{
     S_file *sc = (S_file *) X(mkscanner)(sizeof(S_file), getchr_file);
     sc->f = f;
     return &sc->super;
}

int X(import_wisdom_from_file)(FILE *input_file)
{
     scanner *s = mkscanner_file(input_file);
     planner *plnr = X(the_planner)();
     int ret = plnr->adt->imprt(plnr, s);
     X(scanner_destroy)(s);
     return ret;
}
