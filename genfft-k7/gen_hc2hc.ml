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
open VFpBasics
open AssignmentsToVfpinstrs
open Fft
open Symmetry


let hc2hc_gen n dir =
  let _ = info "generating..." in
  let (zeroth_elements, middle_elements, final_elements) =
    match dir with
      | FORWARD -> 
	  (no_twiddle_gen_expr n real_sym dir,
	   twiddle_dit_gen_expr n no_sym middle_hc2hc_forward_sym dir,
	   no_twiddle_gen_expr (2*n) final_hc2hc_forward_sym dir)
      | BACKWARD ->
	  (no_twiddle_gen_expr n hermitian_sym dir,
	   twiddle_dif_gen_expr n middle_hc2hc_backward_sym no_sym dir,
	   no_twiddle_gen_expr (2*n) final_hc2hc_backward_sym dir)
  and (_, num_twiddles, _) = Twiddle.twiddle_policy () in

  let zeroth_elements' = 
	vect_optimize varinfo_hc2hc_zerothelements n zeroth_elements
  and middle_elements' = 
	vect_optimize varinfo_hc2hc_middleelements n middle_elements
  and final_elements'  = 
	vect_optimize varinfo_hc2hc_finalelements n final_elements in

  let fnarg_inout, fnarg_iostride = K7_MFunArg 1,K7_MFunArg 3
  and fnarg_w,fnarg_m,fnarg_dist = K7_MFunArg 2,K7_MFunArg 4,K7_MFunArg 5 in

  let (inout,iostride1p,iostride4p) = makeNewVintreg3 () 
  and (w,x,y) = makeNewVintreg3 () in

  let initcode = 
	[(iostride4p, [K7V_IntLoadEA(K7V_SID(iostride1p,4,0), iostride4p)])] @
	loadfnargs [(fnarg_inout, inout); 
		    (fnarg_w, w); 
		    (fnarg_iostride, iostride1p)] in
  let initcode' = map (fun (d,xs) -> AddIntOnDemandCode(d,xs)) initcode in

  let (unparser_1, unparser_2) = 
    match dir with
      | FORWARD  -> (strided_hc2hc_unparser_1, strided_hc2hc_unparser_2)
      | BACKWARD -> (strided_hc2hc_unparser_2, strided_hc2hc_unparser_1) in

  let unparser_zeroth_elements =
	make_asm_unparser
	  ([], unparser_1 (inout,inout,iostride4p,n))
	  ([], unparser_2 (inout,inout,iostride4p,n))
	  ([], fail_unparser "hc2hc unparser zeroth elements (TWIDDLE)")
  and unparser_middle_elements =
	make_asm_unparser
	  ([], unparser_1 (x,y,iostride4p,n))
	  ([], unparser_2 (x,y,iostride4p,n))
	  ([], unitstride_complex_unparser w)
  and unparser_final_elements =
	make_asm_unparser
	  ([], unparser_1 (x,y,iostride4p,n))
	  ([], unparser_2 (x,y,iostride4p,n))
	  ([], fail_unparser "hc2hc unparser final elements (TWIDDLE)") in

  let k7vinstrs' = 
	[
	 K7V_IntUnaryOpMem(K7_IShlImm 2, fnarg_dist)	(* dist *= 4 *)
	]  @
	(vsimdinstrsToK7vinstrs unparser_zeroth_elements zeroth_elements') @
	[
	 K7V_RefInts([w]);
	 K7V_IntCpyUnaryOp(K7_ICopy, inout, x);
	 K7V_IntCpyUnaryOp(K7_ICopy, inout, y);
	 K7V_Jump(K7V_BTarget_Named(".LC2"));

	 K7V_Label(".LC1")
	] @
	(vsimdinstrsToK7vinstrs unparser_middle_elements middle_elements') @
	[
	 K7V_IntUnaryOp(K7_IAddImm(num_twiddles n * 8), w);
	 
	 K7V_IntBinOpMem(K7_IAdd, fnarg_dist, x);
	 K7V_IntBinOpMem(K7_ISub, fnarg_dist, y);
	 K7V_IntUnaryOpMem(K7_ISubImm 2, fnarg_m);
	 K7V_CondBranch(K7_BCond_GreaterZero, K7V_BTarget_Named(".LC1"));
	 K7V_CondBranch(K7_BCond_EqualZero, K7V_BTarget_Named(".LC3"));
	 K7V_Jump(K7V_BTarget_Named(".LC4"));

	 K7V_Label(".LC2");
	 K7V_IntBinOpMem(K7_IAdd, fnarg_dist, x);
	 K7V_IntBinOpMem(K7_ISub, fnarg_dist, y);
	 K7V_IntUnaryOpMem(K7_ISubImm 2, fnarg_m);

	 K7V_RefInts([x;y;w]);
	 K7V_CondBranch(K7_BCond_GreaterZero, K7V_BTarget_Named(".LC1"));
	 K7V_CondBranch(K7_BCond_EqualZero, K7V_BTarget_Named(".LC3"));
	 K7V_Jump(K7V_BTarget_Named(".LC4"));
	 K7V_Label(".LC3")
	] @
	(vsimdinstrsToK7vinstrs unparser_final_elements final_elements') @
	[
	 K7V_Label(".LC4")
	] in
  (n, dir, HC2HC, initcode', k7vinstrs')





