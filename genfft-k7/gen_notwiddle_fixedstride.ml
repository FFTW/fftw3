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

let no_twiddle_gen_with_fixedstride istride ostride n dir =
  let _ = info "generating..." in
  let code = Fft.no_twiddle_gen_expr n Symmetry.no_sym dir in
  let code' = vect_optimize varinfo_notwiddle n code in

  let _ = info "generating k7vinstrs..." in
  let (fnarg_input, fnarg_output) = (K7_MFunArg 1, K7_MFunArg 2) in
  let (input,output) = makeNewVintreg2 () in
  let int_initcode = 
	loadfnargs [(fnarg_input, input); (fnarg_output, output)] in
  let initcode = map (fun (d,xs) -> AddIntOnDemandCode(d,xs)) int_initcode in
  let (in_unparser',out_unparser') =
      (([], fixedstride_complex_unparser istride input),
       ([], fixedstride_complex_unparser ostride output)) in
  let unparser = make_asm_unparser_notwiddle in_unparser' out_unparser' in
    (n, dir, NO_TWIDDLE, initcode, vsimdinstrsToK7vinstrs unparser code')
