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

let no_twiddle_gen n dir =
  let _ = info "generating..." in
  let code = Fft.no_twiddle_gen_expr n Symmetry.no_sym dir in
  let code' = vect_optimize varinfo_notwiddle n code in

  let _ = info "generating k7vinstrs..." in
  let (fnarg_input,  fnarg_istride) = (K7_MFunArg 1, K7_MFunArg 3)
  and (fnarg_output, fnarg_ostride) = (K7_MFunArg 2, K7_MFunArg 4) in

  let (input,input2), (output,output2) = makeNewVintreg2 (), makeNewVintreg2 ()
  and (istride8p, istride8n) = makeNewVintreg2 ()
  and (ostride8p, ostride8n) = makeNewVintreg2 () in
  let int_initcode = 
	loadfnargs [(fnarg_input, input); (fnarg_output, output)] @
        [
         (input2,    get2ndhalfcode input  istride8p input2  (pred (msb n)));
         (output2,   get2ndhalfcode output ostride8p output2 (pred (msb n)));
	 (istride8p, [K7V_IntLoadMem(fnarg_istride,istride8p);
		      K7V_IntLoadEA(K7V_SID(istride8p,8,0), istride8p)]);
         (ostride8p, [K7V_IntLoadMem(fnarg_ostride,ostride8p);
		      K7V_IntLoadEA(K7V_SID(ostride8p,8,0), ostride8p)]);
         (istride8n, [K7V_IntCpyUnaryOp(K7_ICopy, istride8p, istride8n);
		      K7V_IntUnaryOp(K7_INegate, istride8n)]);
         (ostride8n, [K7V_IntCpyUnaryOp(K7_ICopy, ostride8p, ostride8n);
		      K7V_IntUnaryOp(K7_INegate, ostride8n)]);
	] in
  let initcode = map (fun (d,xs) -> AddIntOnDemandCode(d,xs)) int_initcode in
  let do_split = n >= 16 in
  let (in_unparser',out_unparser') =
    if do_split then
      let splitPt = 1 lsl (pred (msb n)) in
        (([K7V_RefInts [istride8p; istride8n]],
          strided_complex_split2_unparser (input,input2,splitPt,istride8p)),
	 ([K7V_RefInts [ostride8p; ostride8n]],
	  strided_complex_split2_unparser (output,output2,splitPt,ostride8p)))
    else
      (([], strided_complex_unparser (input,istride8p)),
       ([], strided_complex_unparser (output,ostride8p))) in
  let unparser = make_asm_unparser_notwiddle in_unparser' out_unparser' in
    (n, dir, NO_TWIDDLE, initcode, vsimdinstrsToK7vinstrs unparser code')
