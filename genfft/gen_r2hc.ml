(*
 * Copyright (c) 1997-1999 Massachusetts Institute of Technology
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
 *)
(* $Id: gen_r2hc.ml,v 1.10 2003-03-02 12:11:56 athena Exp $ *)

open Util
open Genutil
open C

let cvsid = "$Id: gen_r2hc.ml,v 1.10 2003-03-02 12:11:56 athena Exp $"

let usage = "Usage: " ^ Sys.argv.(0) ^ " -n <number>"

let uistride = ref Stride_variable
let urostride = ref Stride_variable
let uiostride = ref Stride_variable
let uivstride = ref Stride_variable
let uovstride = ref Stride_variable
let dftII_flag = ref false

let speclist = [
  "-with-istride",
  Arg.String(fun x -> uistride := arg_to_stride x),
  " specialize for given input stride";

  "-with-rostride",
  Arg.String(fun x -> urostride := arg_to_stride x),
  " specialize for given real output stride";

  "-with-iostride",
  Arg.String(fun x -> uiostride := arg_to_stride x),
  " specialize for given imaginary output stride";

  "-with-ivstride",
  Arg.String(fun x -> uivstride := arg_to_stride x),
  " specialize for given input vector stride";

  "-with-ovstride",
  Arg.String(fun x -> uovstride := arg_to_stride x),
  " specialize for given output vector stride";

  "-dft-II",
  Arg.Unit(fun () -> dftII_flag := true),
  " produce shifted dftII-style codelets"
] 

let rdftII sign n input =
  let input' i = if i < n then input i else Complex.zero in
  let f = Fft.dft sign (2 * n) input' in
  let g i = f (2 * i + 1)
  in fun i -> 
    if (i < n - i) then g i
    else if (2 * i + 1 == n) then Complex.real (g i)
    else Complex.zero

let generate n =
  let iarray = "I"
  and roarray = "ro"
  and ioarray = "io"
  and istride = "is"
  and rostride = "ros" 
  and iostride = "ios" 
  and (transform, kind) =
    if !dftII_flag then(rdftII, "r2hcII") else (Trig.rdft, "r2hc")
  in

  let ns = string_of_int n
  and sign = !Genutil.sign 
  and name = !Magic.codelet_name in
  let name0 = name ^ "_0" in

  let vistride = either_stride (!uistride) (C.SVar istride)
  and vrostride = either_stride (!urostride) (C.SVar rostride)
  and viostride = either_stride (!uiostride) (C.SVar iostride)
  in

  let _ = Simd.ovs := stride_to_string "ovs" !uovstride in
  let _ = Simd.ivs := stride_to_string "ivs" !uivstride in

  let locations = unique_array_c n in
  let input = 
    locative_array_c n 
      (C.array_subscript iarray vistride)
      (C.array_subscript "BUG" vistride)
      locations in
  let output = transform sign n (load_array_r n input) in
  let oloc = 
    locative_array_c n 
      (C.array_subscript roarray vrostride)
      (C.array_subscript ioarray viostride)
      locations in
  let odag = store_array_hc n oloc output in
  let annot = standard_optimizer odag in

  let tree0 =
    Fcn ("static MAYBE_INLINE void", name0,
	 ([Decl (C.constrealtypep, iarray);
	   Decl (C.realtypep, roarray);
	   Decl (C.realtypep, ioarray)]
	  @ (if stride_fixed !uistride then [] 
               else [Decl (C.stridetype, istride)])
	  @ (if stride_fixed !urostride then [] 
	       else [Decl (C.stridetype, rostride)])
	  @ (if stride_fixed !uiostride then [] 
	       else [Decl (C.stridetype, iostride)])
	  @ (choose_simd []
	       (if stride_fixed !uivstride then [] else 
	       [Decl ("int", !Simd.ivs)]))
	  @ (choose_simd []
	       (if stride_fixed !uovstride then [] else 
	       [Decl ("int", !Simd.ovs)]))
	 ),
	 add_constants (Asch annot))

  in let loop =
    "static void " ^ name ^
    "(const " ^ C.realtype ^ " *I, " ^ 
    C.realtype ^ " *ro, " ^
    C.realtype ^ " *io, " ^
    C.stridetype ^ " is, " ^ 
    C.stridetype ^ " ros, " ^ 
    C.stridetype ^ " ios, " ^ 
      " int v, int ivs, int ovs)\n" ^
    "{\n" ^
    "int i;\n" ^
    "for (i = 0; i < v; ++i) {\n" ^
      name0 ^ "(I, ro, io" ^
       (if stride_fixed !uistride then "" else ", is") ^ 
       (if stride_fixed !urostride then "" else ", ros") ^ 
       (if stride_fixed !uiostride then "" else ", ios") ^ 
       (choose_simd ""
	  (if stride_fixed !uivstride then "" else ", ivs")) ^ 
       (choose_simd ""
	  (if stride_fixed !uovstride then "" else ", ovs")) ^ 
    ");\n" ^
    "I += ivs; ro += ovs; io += ovs;\n" ^
    "}\n}\n\n"

  and desc = 
    Printf.sprintf 
      "static const kr2hc_desc desc = { %d, \"%s\", %s, &GENUS, %s, %s, %s, %s, %s };\n\n"
      n name (flops_of tree0) 
      (stride_to_solverparm !uistride) 
      (stride_to_solverparm !urostride) (stride_to_solverparm !uiostride)
      (choose_simd "0" (stride_to_solverparm !uivstride))
      (choose_simd "0" (stride_to_solverparm !uovstride))

  and init =
    (declare_register_fcn name) ^
    "{" ^
    "  X(k" ^ kind ^ "_register)(p, " ^ name ^ ", &desc);\n" ^
    "}\n"

  in
  (unparse cvsid tree0) ^ "\n" ^ loop ^ desc ^ init


let main () =
  begin
    parse speclist usage;
    print_string (generate (check_size ()));
  end

let _ = main()
