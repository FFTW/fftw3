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

/* internal API definitions */
#ifndef __API_H__
#define __API_H__

#undef _Complex_I  /* just in case: force <fftw3.h> not to use C99
		      complex numbers */

#include "fftw3.h"
#include "ifftw.h"

/* the API ``plan'' contains both the kernel plan and problem */
struct X(plan_s) {
     plan *pln;
     problem *prb;
};

/* shorthand */
typedef struct X(plan_s) apiplan;

/* complex type for internal use */
typedef R C[2];

void X(extract_reim)(int sign, C *c, R **r, R **i);

planner *X(the_planner)(void);
void X(configure_planner)(planner *plnr);

void X(mapflags)(planner *, unsigned int);

apiplan *X(mkapiplan)(unsigned int flags, problem *prb);

#endif /* __API_H__ */
