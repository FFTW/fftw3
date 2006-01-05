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

/* $Id: getopt-utils.c,v 1.4 2006-01-05 03:04:27 stevenj Exp $ */
#include "bench.h"
#include "getopt.h"
#include <ctype.h>
#include <stdio.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

/* make a short option string for getopt from the long option description */
char *make_short_options(const struct option *opt)
{
    int nopt;
    const struct option *p;
    char *s, *t;

    nopt = 0;
    for (p = opt; p->name; ++p)
	++nopt;

    t = s = (char *) bench_malloc(3 * nopt + 1);

    for (p = opt; p->name; ++p) {
	if (!(p->flag) && isprint(p->val)) {
	    *s++ = p->val;
	    switch (p->has_arg) {
	    case no_argument:
		break;
	    case required_argument:
		*s++ = ':';
		break;
	    case optional_argument:
		*s++ = ':';
		*s++ = ':';
		break;
	    }
	}
    }

    *s++ = '\0';
    return t;
}


/*
 * print usage string.  Derived from lib/display.c in the
 * GNU plotutils v2.4.1.  Distributed under the GNU GPL.
 */
void usage(const char *progname, const struct option *opt)
{
    int i;
    int col = 0;

    fprintf(stdout, "Usage: %s", progname);
    col += (strlen(progname) + 7);
    for (i = 0; opt[i].name; i++) {
	int option_len;

	option_len = strlen(opt[i].name);
	if (col >= 80 - (option_len + 16)) {
	    fputs("\n\t", stdout);
	    col = 8;
	}
	fprintf(stdout, " [--%s", opt[i].name);
	col += (option_len + 4);
	if ((int) (opt[i].val) < 256) {
	    fprintf(stdout, " | -%c", opt[i].val);
	    col += 5;
	}
	if (opt[i].has_arg == required_argument) {
	    fputs(" arg]", stdout);
	    col += 5;
	} else if (opt[i].has_arg == optional_argument) {
	    fputs(" [arg(s)]]", stdout);
	    col += 10;
	} else {
	    fputs("]", stdout);
	    col++;
	}
    }

    fputs ("\n", stdout);
}
