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

#define MAP_IS_MASK (1U << 31) /* indicates a mapping is an & mask, not an | */

/* Apply flagmap to map iflags to oflags, returning the modified oflags.
   If !inv, then only apply mappings whose antecedent is in iflags;
   otherwise, apply the inverse mapping for missing iflags. */
static uint do_map(uint iflags, uint oflags, const uint flagmap[][2], int nmap,
		   int inv)
{
     int i;

     for (i = 0; i < nmap; ++i) {
	  if (iflags & flagmap[i][0]) {
	       if (MAP_IS_MASK & flagmap[i][1])
		    oflags &= flagmap[i][1];
	       else
		    oflags |= flagmap[i][1];
	  }
	  else if (inv) {
	       if (MAP_IS_MASK & flagmap[i][1])
		    oflags |= ~flagmap[i][1];
	       else
		    oflags &= ~flagmap[i][1];
	  }
     }
     return oflags;
}

#define NMAP(flagmap) (sizeof(flagmap) / (2 * sizeof(uint)))

void X(mapflags)(planner *plnr, unsigned int flags)
{
     /* map of api flags -> api flags, to implement consistency rules
	and combination flags */
     const uint self_flagmap[][2] = {
	  /* in some cases (notably for halfcomplex->real transforms),
	     destroy_input is the default, so we need to support
	     an inverse flag to disable it: */
	  { FFTW_PRESERVE_INPUT, ~FFTW_DESTROY_INPUT },

	  { FFTW_EXHAUSTIVE, FFTW_ALLOW_UGLY }, /* exhaustive includes ugly */

	  { FFTW_ESTIMATE, (FFTW_ESTIMATE_PATIENT
			    | FFTW_IMPATIENT /* map is processed in order */
			    | FFTW_NO_INDIRECT_OP) },
	  
	  /* a canonical set of fftw2-like impatient flags */
	  { FFTW_IMPATIENT, (FFTW_NO_VRECURSE
			     | FFTW_NO_RANK_SPLITS
			     | FFTW_NO_VRANK_SPLITS
			     | FFTW_NONTHREADED_ICKY
			     | FFTW_DFT_R2HC_ICKY
			     | FFTW_BELIEVE_PCOST) }
     };
     
     /* map of (processed) api flags to internal problem/planner flags */
     const uint problem_flagmap[][2] = {
	  { FFTW_DESTROY_INPUT, DESTROY_INPUT },
	  { FFTW_POSSIBLY_UNALIGNED, POSSIBLY_UNALIGNED },
	  { FFTW_CONSERVE_MEMORY, CONSERVE_MEMORY },
     };
     const uint planner_flagmap[][2] = {
	  { FFTW_EXHAUSTIVE, ~NO_EXHAUSTIVE },

	  /* the following are undocumented, "beyond-guru" flags that
	     require some understanding of FFTW internals */
	  { FFTW_ESTIMATE_PATIENT, ESTIMATE },
	  { FFTW_BELIEVE_PCOST, BELIEVE_PCOST },
	  { FFTW_DFT_R2HC_ICKY, DFT_R2HC_ICKY },
	  { FFTW_NONTHREADED_ICKY, NONTHREADED_ICKY },
	  { FFTW_NO_BUFFERING, NO_BUFFERING },
	  { FFTW_NO_INDIRECT_OP, NO_INDIRECT_OP },
	  { FFTW_ALLOW_LARGE_GENERIC, ~NO_LARGE_GENERIC },
	  { FFTW_NO_RANK_SPLITS, NO_RANK_SPLITS },
	  { FFTW_ALLOW_UGLY, ~NO_UGLY },
	  { FFTW_NO_VRANK_SPLITS, NO_VRANK_SPLITS },
	  { FFTW_NO_VRECURSE, NO_VRECURSE }
     };

     flags = do_map(flags, flags, self_flagmap, NMAP(self_flagmap), 0);

     plnr->problem_flags = do_map(flags, 0, problem_flagmap, 
				  NMAP(problem_flagmap), 1);

     plnr->planner_flags = do_map(flags, 0, planner_flagmap, 
				  NMAP(planner_flagmap), 1);
}
