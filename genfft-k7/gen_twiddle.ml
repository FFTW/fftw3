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
open Complex

let cvsid = "$Id: gen_twiddle.ml,v 1.4 2002-06-16 22:30:18 athena Exp $"
let usage = "Usage: " ^ Sys.argv.(0) ^ " -n <number>"


let twiddle_gen n sign nt byw =
  let _ = info "generating..." in
  let expr = Fft.dft sign n (byw (load_var @@ access_input)) in
  let code = store_array_c n expr in
  let code' = vect_optimize varinfo_twiddle n code in

  let _ = info "generating k7vinstrs..." in
  let fnarg_inout,fnarg_iostride  = K7_MFunArg 1, K7_MFunArg 3
  and fnarg_w,fnarg_m,fnarg_idist = K7_MFunArg 2,K7_MFunArg 4,K7_MFunArg 5 in

  let (inout,inout2) = makeNewVintreg2 ()
  and (iostride4p,iostride4n,idist4p) = makeNewVintreg3 ()
  and (w, m) = makeNewVintreg2 () in

  let int_initcode = 
	loadfnargs [(fnarg_inout, inout); (fnarg_w, w); (fnarg_m, m)] @
	[
	 (inout2,     get2ndhalfcode inout iostride4p inout2 (pred (msb n)));
	 (idist4p,    [K7V_IntLoadMem(fnarg_idist, idist4p);
		       K7V_IntLoadEA(K7V_SID(idist4p,4,0), idist4p)]);
	 (iostride4p, [K7V_IntLoadMem(fnarg_iostride,iostride4p);
		       K7V_IntLoadEA(K7V_SID(iostride4p,4,0), iostride4p)]);
	] in
  let initcode = map (fun (d,xs) -> AddIntOnDemandCode(d,xs)) int_initcode in
  let do_split = n >= 32 in
  let io_unparser' =
	if do_split then
          ([],
	   strided_complex_split2_unparser 
		(inout,inout2,1 lsl (pred (msb n)),iostride4p))
	else
	  ([], strided_complex_unparser (inout,iostride4p)) in
  let tw_unparser' = ([], unitstride_complex_unparser w) in
  let unparser = make_asm_unparser io_unparser' io_unparser' tw_unparser' in
  let body = 
    if do_split then
	[
	 K7V_RefInts([inout; inout2; w; iostride4p]);
	 K7V_IntUnaryOpMem(K7_IShlImm 2, fnarg_idist);
	 K7V_Label(".L0")
	] @
	(vsimdinstrsToK7vinstrs unparser code') @
	[
	 K7V_IntUnaryOp(K7_IAddImm(nt * 4), w);
	 K7V_IntBinOpMem(K7_IAdd, fnarg_idist, inout);
	 K7V_IntBinOpMem(K7_IAdd, fnarg_idist, inout2);
	 K7V_IntUnaryOpMem(K7_IDec, fnarg_m);
	 K7V_RefInts([inout; inout2; w; iostride4p]);
	 K7V_CondBranch(K7_BCond_NotZero, K7V_BTarget_Named ".L0")
	]
    else
	[
	 K7V_RefInts([inout; w; iostride4p; idist4p; m]);
	 K7V_Label(".L0");
	] @
	(vsimdinstrsToK7vinstrs unparser code') @
	[
	 K7V_IntUnaryOp(K7_IAddImm(nt * 4), w);
	 K7V_IntBinOp(K7_IAdd, idist4p, inout);
	 K7V_IntUnaryOp(K7_IDec, m);
	 K7V_RefInts([inout; w; iostride4p; idist4p; m]);
	 K7V_CondBranch(K7_BCond_NotZero, K7V_BTarget_Named ".L0")
	]
  in ((initcode, body), k7vFlops body)


let generate n =
  let name = !Magic.codelet_name
  and sign = !GenUtil.sign
  in
  let (bytwiddle, num_twiddles, twdesc) = Twiddle.twiddle_policy () in
  let nt = num_twiddles n in
  let byw = bytwiddle n sign (load_constant_array_c nt) in

  let (code, (add, mul)) = twiddle_gen n sign nt byw in
  let p = Printf.printf in
  begin
    boilerplate cvsid;
    p "#if defined(FFTW_SINGLE) && defined(K7_MODE)\n";
    compileToAsm name code;
    p "\n";
    p ".section .rodata\n";
    p "\t.align 2\n";
    p "twinstr:\n";
    p "%s" (Twiddle.twinstr_to_asm_string (twdesc n));
    p "\t.align 4\n";
    p "desc:\n";
    p "\t.long %d\n" n;
    p "\t.long %d\n" sign;
    p "\t.long twinstr\n";
    p "\t.long 0\n";  (* is *)
    p "\t.long 0\n";  (* ivs *)
    p "\t.long %d\n" add;
    p "\t.long %d\n" mul;
    p "\t.long 0\n";  (* fma *)
    p "\t.long 0\n";  (* other *)
    p "\n";
    p ".text\n";
    p "\t.align 4\n";
    p ".globl %s\n" (register_fcn name);
    p "%s:\n" (register_fcn name);
    p "\tsubl $12,%%esp\n";
    p "\tmovl 16(%%esp),%%eax\n";
    p "\taddl $-4,%%esp\n";
    p "\tpushl $desc\n";
    p "\tpushl $%s\n" name;
    p "\tpushl %%eax\n";
    p "\tcall sfftw_kdft_dit_k7_register\n";
    p "\taddl $16,%%esp\n";
    p "\taddl $12,%%esp\n";
    p "\tret\n";
    p "\n";
    Printf.printf "#endif\n";
  end

let main () =
  begin
    parse speclist usage;
    generate (check_size());
  end

let _ = main()
