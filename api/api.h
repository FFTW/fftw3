/*
 * Copyright (c) 2003, 2007-14 Matteo Frigo
 * Copyright (c) 2003, 2007-14 Massachusetts Institute of Technology
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/* internal API definitions */
#ifndef __API_H__
#define __API_H__

/* just in case: force <fftw3.h> not to use C99 complex numbers
   (we need this for IBM xlc because _Complex_I is treated specially
   and is defined even if <complex.h> is not included) */
#define FFTW_NO_Complex

#include "api/fftw3.h"
#include "kernel/ifftw.h"
#include "rdft/rdft.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* the API ``plan'' contains both the kernel plan and problem */
struct X(plan_s) {
     plan *pln;
     problem *prb;
     int sign;
};

/* shorthand */
typedef struct X(plan_s) apiplan;

/* complex type for internal use */
typedef R C[2];

#define EXTRACT_REIM(sign, c, r, i) X(extract_reim)(sign, (c)[0], r, i)

#define TAINT_UNALIGNED(p, flg) TAINT(p, ((flg) & FFTW_UNALIGNED) != 0)

tensor *X(mktensor_rowmajor)(int rnk, const int *n,
			     const int *niphys, const int *nophys,
			     int is, int os);

tensor *X(mktensor_iodims)(int rank, const X(iodim) *dims, int is, int os);
tensor *X(mktensor_iodims64)(int rank, const X(iodim64) *dims, int is, int os);
const int *X(rdft2_pad)(int rnk, const int *n, const int *nembed,
			int inplace, int cmplx, int **nfree);

int X(many_kosherp)(int rnk, const int *n, int howmany);
int X(guru_kosherp)(int rank, const X(iodim) *dims,
		    int howmany_rank, const X(iodim) *howmany_dims);
int X(guru64_kosherp)(int rank, const X(iodim64) *dims,
		    int howmany_rank, const X(iodim64) *howmany_dims);

/* Note: extern is used for "internal" functions used in tests/hook.c */

FFTW3_EXPORT printer *X(mkprinter_file)(FILE *f);

printer *X(mkprinter_cnt)(size_t *cnt);
printer *X(mkprinter_str)(char *s);

FFTW3_EXPORT planner *X(the_planner)(void);
void X(configure_planner)(planner *plnr);

void X(mapflags)(planner *, unsigned);

apiplan *X(mkapiplan)(int sign, unsigned flags, problem *prb);

rdft_kind *X(map_r2r_kind)(int rank, const X(r2r_kind) * kind);

typedef void (*planner_hook_t)(void);
                                                     
void EXPORT_ADDITIONAL_FUNCTIONS X(set_planner_hooks)(planner_hook_t before, planner_hook_t after);

#ifdef __cplusplus
}  /* extern "C" */
#endif /* __cplusplus */

#endif				/* __API_H__ */
