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


let realodd2_gen n =
  let _ = info "generating..." in
  let code  = no_twiddle_gen_expr (2*n) Symmetry.realodd2_input_sym FORWARD in
  let code' = vect_optimize varinfo_realodd n code in

  let _ = info "generating k7vinstrs..." in
  let (fnarg_input, fnarg_istride)  = (K7_MFunArg 1, K7_MFunArg 3)
  and (fnarg_output, fnarg_ostride) = (K7_MFunArg 2, K7_MFunArg 4) in

  let (input, istride1p, istride4p) = makeNewVintreg3 ()
  and (output, ostride1p, ostride4p) = makeNewVintreg3 () in

  let initcode = 
	[(istride4p, [K7V_IntLoadEA(K7V_SID(istride1p,4,0), istride4p)]);
         (ostride4p, [K7V_IntLoadEA(K7V_SID(ostride1p,4,0), ostride4p)])] @
	loadfnargs [(fnarg_input, input);   (fnarg_istride, istride1p);
		    (fnarg_output, output); (fnarg_ostride, ostride1p)] in

  let initcode' = map (fun (d,xs) -> AddIntOnDemandCode(d,xs)) initcode in

  let in_unparser' =
	([], 
	 strided_realofcomplex_unparser_withoffset (input,istride4p,0))
  and out_unparser' =
	([], 
	 strided_imagofcomplex_unparser_withoffset (output,ostride4p,-1)) in
  let unparser = make_asm_unparser_notwiddle in_unparser' out_unparser' in
    (n, FORWARD, REALODD2, initcode', vsimdinstrsToK7vinstrs unparser code')
