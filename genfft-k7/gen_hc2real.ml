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
open CodeletMisc
open AssignmentsToVfpinstrs
open Fft

let hc2real_gen n =
  let _ = info "generating..." in
  let code = no_twiddle_gen_expr n Symmetry.hermitian_sym BACKWARD in
  let code' = vect_optimize varinfo_hc2real n code in

  let _ = info "generating k7vinstrs..." in
  let (fnarg_inputRe, fnarg_istrideRe) = (K7_MFunArg 1, K7_MFunArg 4)
  and (fnarg_inputIm, fnarg_istrideIm) = (K7_MFunArg 2, K7_MFunArg 5)
  and (fnarg_output,  fnarg_ostride)   = (K7_MFunArg 3, K7_MFunArg 6) in

  let (inputRe, inputIm) = makeNewVintreg2 () 
  and (output, output2)  = makeNewVintreg2 () 
  and (istrideRe1p, istrideRe4p) = makeNewVintreg2 ()
  and (istrideIm1p, istrideIm4p) = makeNewVintreg2 ()
  and (ostride1p, ostride4p) = makeNewVintreg2 () in

  let int_initcode = 
	loadfnargs [(fnarg_inputRe, inputRe); (fnarg_istrideRe, istrideRe1p);
		    (fnarg_inputIm, inputIm); (fnarg_istrideIm, istrideIm1p);
		    (fnarg_output, output);   (fnarg_ostride, ostride1p)] @
	[
	 (output2,     get2ndhalfcode output ostride4p output2 (pred (msb n)));
	 (istrideRe4p, [K7V_IntLoadEA(K7V_SID(istrideRe1p,4,0), istrideRe4p)]);
	 (istrideIm4p, [K7V_IntLoadEA(K7V_SID(istrideIm1p,4,0), istrideIm4p)]);
	 (ostride4p,   [K7V_IntLoadEA(K7V_SID(ostride1p,4,0), ostride4p)]);
	] in
  let initcode = map (fun (d,xs) -> AddIntOnDemandCode(d,xs)) int_initcode in
  let do_split = n > 16 in
  let in_unparser' =
	([],
	 strided_dualreal_unparser 
		(inputRe,istrideRe4p) (inputIm,istrideIm4p)) in
  let out_unparser' =
	if do_split then
          let splitPt = 1 lsl (pred (msb n)) in
            ([], 
	     strided_real_split2_unparser (output,output2,splitPt,ostride4p))
	else
	  ([], strided_real_unparser (output,ostride4p)) in
  let unparser = make_asm_unparser_notwiddle in_unparser' out_unparser' in
    (n, BACKWARD, HC2REAL, initcode, vsimdinstrsToK7vinstrs unparser code')






