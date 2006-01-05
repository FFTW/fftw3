/*
 * Copyright (c) 2001 Matteo Frigo
 * Copyright (c) 2001 Massachusetts Institute of Technology
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

/* $Id: info.c,v 1.4 2006-01-05 03:04:27 stevenj Exp $ */

#include "bench.h"
#include <stdio.h>
#include <string.h>

void report_info(const char *param)
{
     struct bench_doc *p;

     for (p = bench_doc; p->key; ++p) {
	  if (!strcmp(param, p->key)) {
	       if (!p->val)
		    p->val = p->f();

	       ovtpvt("%s\n", p->val);
	  }
     }
}

void report_info_all(void)
{
     struct bench_doc *p;

     /*
      * TODO: escape quotes?  The format is not unambigously
      * parseable if the info string contains double quotes.
      */
     for (p = bench_doc; p->key; ++p) {
	  if (!p->val)
	       p->val = p->f();
	  ovtpvt("(%s \"%s\")\n", p->key, p->val);
     }
     ovtpvt("(benchmark-precision \"%s\")\n", 
	    SINGLE_PRECISION ? "single" : 
	    (LDOUBLE_PRECISION ? "long-double" : "double"));
}

