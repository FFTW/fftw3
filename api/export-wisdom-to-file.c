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

void X(export_wisdom_to_file)(FILE *output_file)
{
     printer *p = X(mkprinter_file)(output_file);
     planner *plnr = X(the_planner)();
     plnr->adt->exprt(plnr, p);
     X(printer_destroy)(p);
}
