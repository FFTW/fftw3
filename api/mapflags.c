/*
 * Copyright (c) 2003 Matteo Frigo
 * Copyright (c) 2003 Massachusetts Institute of Technology
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

/* a flag operation: x is either a flag, in which case xm == 0, or
   a mask, in which case xm == x; using this we can compactly code
   the various bit operations via (flags & x) ^ xm or (flags | x) ^ xm. */
typedef struct {
     unsigned x, xm;
} flagmask;

typedef struct {
     flagmask flag;
     flagmask op;
} flagop;

#define FLAGP(f, msk)(((f) & (msk).x) ^ (msk).xm)
#define OP(f, msk)(((f) | (msk).x) ^ (msk).xm)

#define YES(x) {x, 0}
#define NO(x) {x, x}
#define IMPLIES(predicate, consequence) { predicate, consequence }
#define EQV(a, b) IMPLIES(YES(a), YES(b)), IMPLIES(NO(a), NO(b))
#define NEQV(a, b) IMPLIES(YES(a), NO(b)), IMPLIES(NO(a), YES(b))

static void map_flags(unsigned *iflags, unsigned *oflags,
		      const flagop flagmap[], int nmap)
{
     int i;
     for (i = 0; i < nmap; ++i)
          if (FLAGP(*iflags, flagmap[i].flag))
               *oflags = OP(*oflags, flagmap[i].op);
}

#define NELEM(array)(sizeof(array) / sizeof((array)[0]))

void X(mapflags)(planner *plnr, unsigned flags)
{
     unsigned tmpflags;

     /* map of api flags -> api flags, to implement consistency rules
        and combination flags */
     const flagop self_flagmap[] = {
	  /* in some cases (notably for halfcomplex->real transforms),
	     DESTROY_INPUT is the default, so we need to support
	     an inverse flag to disable it: */
	  IMPLIES(YES(FFTW_PRESERVE_INPUT), NO(FFTW_DESTROY_INPUT)),

	  IMPLIES(YES(FFTW_EXHAUSTIVE), YES(FFTW_PATIENT)),

	  IMPLIES(YES(FFTW_ESTIMATE), NO(FFTW_PATIENT)),
	  IMPLIES(YES(FFTW_ESTIMATE),
		  YES(FFTW_ESTIMATE_PATIENT | FFTW_NO_INDIRECT_OP)),

	  /* a canonical set of fftw2-like impatience flags */
	  IMPLIES(NO(FFTW_PATIENT),
		  YES(FFTW_NO_VRECURSE
		      | FFTW_NO_RANK_SPLITS
		      | FFTW_NO_VRANK_SPLITS
		      | FFTW_NONTHREADED_ICKY
		      | FFTW_DFT_R2HC_ICKY
		      | FFTW_BELIEVE_PCOST))
     };

     /* map of (processed) api flags to internal problem/planner flags */
     const flagop problem_flagmap[] = {
	  EQV(FFTW_DESTROY_INPUT, DESTROY_INPUT),
	  EQV(FFTW_NO_SIMD, NO_SIMD),
	  EQV(FFTW_CONSERVE_MEMORY, CONSERVE_MEMORY)
     };
     const flagop planner_flagmap[] = {
	  NEQV(FFTW_EXHAUSTIVE, NO_EXHAUSTIVE),

	  /* the following are undocumented, "beyond-guru" flags that
	     require some understanding of FFTW internals */
	  EQV(FFTW_ESTIMATE_PATIENT, ESTIMATE),
	  EQV(FFTW_BELIEVE_PCOST, BELIEVE_PCOST),
	  EQV(FFTW_DFT_R2HC_ICKY, DFT_R2HC_ICKY),
	  EQV(FFTW_NONTHREADED_ICKY, NONTHREADED_ICKY),
	  EQV(FFTW_NO_BUFFERING, NO_BUFFERING),
	  EQV(FFTW_NO_INDIRECT_OP, NO_INDIRECT_OP),
	  NEQV(FFTW_ALLOW_LARGE_GENERIC, NO_LARGE_GENERIC),
	  EQV(FFTW_NO_RANK_SPLITS, NO_RANK_SPLITS),
	  EQV(FFTW_NO_VRANK_SPLITS, NO_VRANK_SPLITS),
	  EQV(FFTW_NO_VRECURSE, NO_VRECURSE)
     };

     map_flags(&flags, &flags, self_flagmap, NELEM(self_flagmap));

     tmpflags = 0;
     map_flags(&flags, &tmpflags, problem_flagmap, NELEM(problem_flagmap));
     plnr->problem_flags = tmpflags;

     tmpflags = 0;
     map_flags(&flags, &tmpflags, planner_flagmap, NELEM(planner_flagmap));
     plnr->planner_flags = tmpflags;
}
