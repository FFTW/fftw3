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

/* if F77_FUNC is not defined, then we don't know how to mangle identifiers
   for the Fortran linker, and we must omit the f77 API. */
#ifdef F77_FUNC

/*-----------------------------------------------------------------------*/
/* some internal functions used by the f77 api */

/* in fortran, the natural array ordering is column-major, which
   corresponds to reversing the dimensions relative to C's row-major */
static int *reverse_n(int rnk, const int *n)
{
     int *nrev;
     int i;
     A(FINITE_RNK(rnk));
     nrev = MALLOC(sizeof(int) * rnk, PROBLEMS);
     for (i = 0; i < rnk; ++i)
          nrev[rnk - i - 1] = n[i];
     return nrev;
}

/* f77 doesn't have data structures, so we have to pass iodims as
   parallel arrays */
static X(iodim) *make_dims(int rnk, const int *n,
			   const int *is, const int *os)
{
     X(iodim) *dims;
     int i;
     A(FINITE_RNK(rnk));
     dims = MALLOC(sizeof(X(iodim)) * rnk, PROBLEMS);
     for (i = 0; i < rnk; ++i) {
          dims[i].n = n[i];
          dims[i].is = is[i];
          dims[i].os = os[i];
     }
     return dims;
}

typedef struct {
     void (*f77_absorber)(char *, void *);
     void *data;
} absorber_data;

static void absorber(char c, void *d)
{
     absorber_data *ad = (absorber_data *) d;
     ad->f77_absorber(&c, ad->data);
}

typedef struct {
     void (*f77_emitter)(int *, void *);
     void *data;
} emitter_data;

static int emitter(void *d)
{
     emitter_data *ed = (emitter_data *) d;
     int c;
     ed->f77_emitter(&c, ed->data);
     return (c < 0 ? EOF : c);
}

/*-----------------------------------------------------------------------*/

/* Fortran-like (e.g. as in BLAS) type prefixes for F77 interface */
#if defined(FFTW_SINGLE)
#  define x77(name) CONCAT(sfftw_, name)
#  define X77(NAME) CONCAT(SFFTW_, NAME)
#elif defined(FFTW_LDOUBLE)
/* FIXME: what is best?  BLAS uses D..._X, apparently.  Ugh. */
#  define x77(name) CONCAT(lfftw_, name)
#  define X77(NAME) CONCAT(LFFTW_, NAME)
#else
#  define x77(name) CONCAT(dfftw_, name)
#  define X77(NAME) CONCAT(DFFTW_, NAME)
#endif

#define F77x(a, A) F77_FUNC(a, A)
#define F77(a, A) F77x(x77(a), X77(A))
#include "f77funcs.c"

/* If identifiers with underscores are mangled differently than those
   without underscores, then we include *both* mangling versions.  The
   reason is that the only Fortran compiler that does such differing
   mangling is currently g77 (which adds an extra underscore to names
   with underscores), whereas other compilers running on the same
   machine are likely to use g77's non-underscored mangling.  (I'm sick
   of users complaining that FFTW works with g77 but not with e.g.
   pgf77 or ifc on the same machine.)  Note that all FFTW identifiers
   contain underscores, and configure picks g77 by default. */
#if defined(F77_FUNC_) && !defined(F77_FUNC_EQUIV)
#  undef F77x
#  define F77x(a, A) F77_FUNC_(a, A)
#  include "f77funcs.c"
#endif

#endif				/* F77_FUNC */
