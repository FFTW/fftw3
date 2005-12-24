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

let cvsid = "$Id: gen_twiddle.ml,v 1.14 2005-12-24 21:08:49 athena Exp $"

type ditdif = DIT | DIF
let ditdif = ref DIT
let usage = "Usage: " ^ Sys.argv.(0) ^ " -n <number> [ -dit | -dif ]"

let speclist = [
  "-dit",
  Arg.Unit(fun () -> ditdif := DIT),
  " generate a DIT codelet";

  "-dif",
  Arg.Unit(fun () -> ditdif := DIF),
  " generate a DIF codelet";
]

let choose sign m p = if sign > 0 then p else m 

let twiddle_gen n sign nt byw =
  let _ = info "generating..." in
  let expr = 
    match !ditdif with
    | DIT -> Fft.dft sign n (byw (load_var @@ access_input)) 
    | DIF -> byw (Fft.dft sign n (load_var @@ access_input)) 
  in
  let code = store_array_c n expr in
  let code' = vect_optimize varinfo_twiddle n code in

  let _ = info "generating k7vinstrs..." in
  let fnarg_inout = choose sign (K7_MFunArg 1) (K7_MFunArg 2)
  and fnarg_w = K7_MFunArg 3
  and fnarg_iostride = K7_MFunArg 4 
  and fnarg_m = K7_MFunArg 5 
  and fnarg_idist = K7_MFunArg 6 in 

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
     (* force W to be allocated in retval register *)
  let initcode = initcode @ [FixRegister (w, retval)] in
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
	 K7V_CondBranch(K7_BCond_NotZero, K7V_BTarget_Named ".L0");
	 K7V_RefInts([w]);
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
    compileToAsm name 6 code;
    p "\n";
    p ".section .rodata\n";
    p "nam:\n";
    p "\t.string \"%s\"\n" name;
    p "\t.align 4\n";
    p "twinstr:\n";
    p "%s" (Twiddle.twinstr_to_asm_string (twdesc n));
    p "\t.align 4\n";
    p "desc:\n";
    p "\t.long %d\n" n;
    p "\t.long nam\n";
    p "\t.long twinstr\n";
    p "\t.long fftwf_kdft_ct_k7_%sgenus\n" (choose sign "m" "p");
    p "\t.double %d\n" add;
    p "\t.double %d\n" mul;
    p "\t.double 0\n";  (* fma *)
    p "\t.double 0\n";  (* other *)
    p "\t.long 0\n";  (* s1 *)
    p "\t.long 0\n";  (* s2 *)
    p "\t.long 0\n";  (* dist *)
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
    begin
      match !ditdif with
      | DIT -> p "\tcall fftwf_kdft_dit_register\n"
      | DIF -> p "\tcall fftwf_kdft_dif_register\n"
    end;
    p "\taddl $16,%%esp\n";
    p "\taddl $12,%%esp\n";
    p "\tret\n";
    p "\n";
  end

let main () =
  begin
    parse (speclist @ Twiddle.speclist) usage;
    generate (check_size());
  end

let _ = main()
