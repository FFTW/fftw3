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

#include "dft.h"

typedef void (*dftwapply) (const plan *ego, R *rio, R *iio);
typedef struct ct_solver_s ct_solver;
typedef plan *(*ct_mkinferior)(const ct_solver *ego,
			    int dec, INT r, INT m, INT s, INT vl, INT vs, 
			    INT mstart, INT mcount,
			    R *rio, R *iio,
			    planner *plnr);

typedef struct {
     plan super;
     dftwapply apply;
} plan_dftw;

extern plan *X(mkplan_dftw)(size_t size, const plan_adt *adt, dftwapply apply);

#define MKPLAN_DFTW(type, adt, apply) \
  (type *)X(mkplan_dftw)(sizeof(type), adt, apply)

struct ct_solver_s {
     solver super;
     INT r;
     int dec;
#    define DECDIF 0
#    define DECDIT 1

     ct_mkinferior mkcldw;
};

int X(ct_applicable)(const ct_solver *, const problem *, planner *);
ct_solver *X(mksolver_ct)(size_t size, INT r, int dec, ct_mkinferior mkcldw);
extern ct_solver *(*X(mksolver_ct_hook))(size_t, INT, int, ct_mkinferior);

void X(regsolver_ct_directw)(planner *plnr,
     kdftw codelet, const ct_desc *desc, int dec);
void X(regsolver_ct_directwbuf)(planner *plnr,
     kdftw codelet, const ct_desc *desc, int dec);
solver *X(mksolver_ctsq)(kdftwsq codelet, const ct_desc *desc, int dec);
