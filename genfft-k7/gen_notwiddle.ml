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
open AssignmentsToVfpinstrs

let no_twiddle_gen n dir =
  let _ = info "generating..." in
  let code = Fft.no_twiddle_gen_expr n Symmetry.no_sym dir in
  let code' = vect_optimize varinfo_notwiddle n code in

  let _ = info "generating k7vinstrs..." in
  let (fnarg_input,  fnarg_istride) = (K7_MFunArg 1, K7_MFunArg 3)
  and (fnarg_output, fnarg_ostride) = (K7_MFunArg 2, K7_MFunArg 4) 
  and fnarg_vl = K7_MFunArg 5
  and fnarg_ivs = K7_MFunArg 6
  and fnarg_ovs = K7_MFunArg 7
  in

  let (input,input2), (output,output2) = makeNewVintreg2 (), makeNewVintreg2 ()
  and (istride4p, ostride4p) = makeNewVintreg2 () in
  let int_initcode = 
	loadfnargs [(fnarg_input, input); (fnarg_output, output)] @
        [
         (input2,    get2ndhalfcode input  istride4p input2  (pred (msb n)));
         (output2,   get2ndhalfcode output ostride4p output2 (pred (msb n)));
	 (istride4p, [K7V_IntLoadMem(fnarg_istride,istride4p);
		      K7V_IntLoadEA(K7V_SID(istride4p,4,0), istride4p)]);
         (ostride4p, [K7V_IntLoadMem(fnarg_ostride,ostride4p);
		      K7V_IntLoadEA(K7V_SID(ostride4p,4,0), ostride4p)]);
	] in
  let initcode = map (fun (d,xs) -> AddIntOnDemandCode(d,xs)) int_initcode in
  let do_split = n >= 16 in
  let (in_unparser',out_unparser') =
    if do_split then
      let splitPt = 1 lsl (pred (msb n)) in
        (([K7V_RefInts [istride4p]],
          strided_complex_split2_unparser (input,input2,splitPt,istride4p)),
	 ([K7V_RefInts [ostride4p]],
	  strided_complex_split2_unparser (output,output2,splitPt,ostride4p)))
    else
      (([], strided_complex_unparser (input,istride4p)),
       ([], strided_complex_unparser (output,ostride4p))) in

  let unparser = make_asm_unparser_notwiddle in_unparser' out_unparser' in

  let body = 
    if do_split then
      [
       K7V_RefInts([input; input2; output; output2; istride4p; ostride4p]);
       K7V_IntUnaryOpMem(K7_IShlImm 2, fnarg_ivs);
       K7V_IntUnaryOpMem(K7_IShlImm 2, fnarg_ovs);
       K7V_Label(".L0")
     ]  @
      (vsimdinstrsToK7vinstrs unparser code') @
      [
       K7V_IntBinOpMem(K7_IAdd, fnarg_ivs, input);
       K7V_IntBinOpMem(K7_IAdd, fnarg_ivs, input2);
       K7V_IntBinOpMem(K7_IAdd, fnarg_ovs, output);
       K7V_IntBinOpMem(K7_IAdd, fnarg_ovs, output2);
       K7V_IntUnaryOpMem(K7_IDec, fnarg_vl);
       K7V_RefInts([input; input2; output; output2; istride4p; ostride4p]);
       K7V_CondBranch(K7_BCond_NotZero, K7V_BTarget_Named ".L0")
     ] 
    else
      [
       K7V_RefInts([input; output; istride4p; ostride4p]);
       K7V_IntUnaryOpMem(K7_IShlImm 2, fnarg_ivs);
       K7V_IntUnaryOpMem(K7_IShlImm 2, fnarg_ovs);
       K7V_Label(".L0")
     ]  @
      (vsimdinstrsToK7vinstrs unparser code') @
      [
       K7V_IntBinOpMem(K7_IAdd, fnarg_ivs, input);
       K7V_IntBinOpMem(K7_IAdd, fnarg_ovs, output);
       K7V_IntUnaryOpMem(K7_IDec, fnarg_vl);
       K7V_RefInts([input; output; istride4p; ostride4p]);
       K7V_CondBranch(K7_BCond_NotZero, K7V_BTarget_Named ".L0")

     ] 
  
  in ((initcode, body), k7vFlops body)

let cvsid = "$Id: gen_notwiddle.ml,v 1.7 2002-06-15 01:11:16 athena Exp $"
let usage = "Usage: " ^ Sys.argv.(0) ^ " -n <number>"

let generate n =
  let name = !Magic.name
  and dir = !GenUtil.dir
  and sign = Fft.sign_of_dir !GenUtil.dir
  in
  let (code, (add, mul)) = no_twiddle_gen n dir in
  begin
    boilerplate cvsid;
    Printf.printf "#if FFTW_SINGLE && K7_MODE\n";
    Printf.printf
      "static void %s(const R *ri, R *ro, int is, int os, uint v, int ivs, int ovs)\n"
      name;
    Printf.printf "{ UNUSED(ri); UNUSED(ro); UNUSED(is); UNUSED(os);\n";
    Printf.printf "  UNUSED(v); UNUSED(ivs); UNUSED(ovs);\n";
    compileToAsm code;
    Printf.printf "}\n\n";
    Printf.printf "static const kdft_k7_desc desc = { %d, %d, { %d, %d, 0, 0} };\n"
      n sign add mul;
    Printf.printf "%s { X(kdft_k7_register)(p, %s, &desc); }"
      (declare_register_fcn name) name;
    Printf.printf "#endif\n";
  end

let main () =
  begin
    parse speclist usage;
    generate (check_size());
  end

let _ = main()
