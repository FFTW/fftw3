(*
 * Copyright (c) 2001 Stefan Kral
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
 *)

open List
open Util
open GenUtil
open VSimdBasics
open K7Basics
open K7RegisterAllocationBasics
open K7Translate
open Fft
open CodeletMisc
open AssignmentsToVfpinstrs

let real2hc_gen n =
  let _ = info "generating..." in
  let code  = no_twiddle_gen_expr n Symmetry.real_sym FORWARD in
  let code' = vect_optimize varinfo_real2hc n code in

  let _ = info "generating k7vinstrs..." in
  let (fnarg_input,    fnarg_istride)   = (K7_MFunArg 1, K7_MFunArg 4)
  and (fnarg_outputRe, fnarg_ostrideRe) = (K7_MFunArg 2, K7_MFunArg 5)
  and (fnarg_outputIm, fnarg_ostrideIm) = (K7_MFunArg 3, K7_MFunArg 6) in

  let (input, input2) = makeNewVintreg2 ()
  and (outputRe, outputIm) = makeNewVintreg2 () 
  and (istride1p, istride4p) = makeNewVintreg2 ()
  and (ostrideRe1p, ostrideRe4p) = makeNewVintreg2 ()
  and (ostrideIm1p, ostrideIm4p) = makeNewVintreg2 () in

  let initcode = 
	loadfnargs [(fnarg_outputRe, outputRe); (fnarg_ostrideRe, ostrideRe1p);
		    (fnarg_outputIm, outputIm); (fnarg_ostrideIm, ostrideIm1p);
		    (fnarg_input, input);       (fnarg_istride, istride1p);] @
	[
	 (input2,      get2ndhalfcode input istride4p input2 (pred (msb n)));
	 (istride4p,   [K7V_IntLoadEA(K7V_SID(istride1p,4,0), istride4p)]);
	 (ostrideRe4p, [K7V_IntLoadEA(K7V_SID(ostrideRe1p,4,0), ostrideRe4p)]);
	 (ostrideIm4p, [K7V_IntLoadEA(K7V_SID(ostrideIm1p,4,0), ostrideIm4p)])
	] in

  let initcode' = map (fun (d,xs) -> AddIntOnDemandCode(d,xs)) initcode in
  let do_split = n > 16 in
  let out_unparser' =
	([],
	 strided_dualreal_unparser 
		(outputRe,ostrideRe4p) (outputIm,ostrideIm4p))
  and in_unparser' =
	if do_split then
          let splitPt = 1 lsl (pred (msb n)) in
            ([], strided_real_split2_unparser (input,input2,splitPt,istride4p))
	else
	  ([], strided_real_unparser (input,istride4p)) in

  let unparser = make_asm_unparser_notwiddle in_unparser' out_unparser' in
    (n, FORWARD, REAL2HC, initcode', vsimdinstrsToK7vinstrs unparser code')
