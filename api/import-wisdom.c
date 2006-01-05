/*
 * Copyright (c) 2003, 2006 Matteo Frigo
 * Copyright (c) 2003, 2006 Massachusetts Institute of Technology
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
     int (*read_char)(void *);
     void *data;
} S;

static int getchr_generic(scanner * s_)
{
     S *s = (S *) s_;
     return (s->read_char)(s->data);
}

int X(import_wisdom)(int (*read_char)(void *), void *data)
{
     S *s = (S *) X(mkscanner)(sizeof(S), getchr_generic);
     planner *plnr = X(the_planner)();
     int ret;

     s->read_char = read_char;
     s->data = data;
     ret = plnr->adt->imprt(plnr, (scanner *) s);
     X(scanner_destroy)((scanner *) s);
     return ret;
}
