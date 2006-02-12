(*
 * Copyright (c) 1997-1999 Massachusetts Institute of Technology
 * Copyright (c) 2003, 2006 Matteo Frigo
 * Copyright (c) 2003, 2006 Massachusetts Institute of Technology
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
(* $Id: gen_r2hc.ml,v 1.18 2006-02-12 23:34:12 athena Exp $ *)

open Util
open Genutil
open C

let cvsid = "$Id: gen_r2hc.ml,v 1.18 2006-02-12 23:34:12 athena Exp $"

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
  and i = "i" 
  and v = "v"
  and (transform, kind) =
    if !dftII_flag then(rdftII, "r2hcII") else (Trig.rdft, "r2hc")
  in

  let sign = !Genutil.sign 
  and name = !Magic.codelet_name in

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

  let body = Block (
    [Decl ("INT", i)],
    [For (Expr_assign (CVar i, CVar v),
	  Binop (" > ", CVar i, Integer 0),
	  list_to_comma 
	    [Expr_assign (CVar i, CPlus [CVar i; CUminus (Integer 1)]);
	     Expr_assign (CVar iarray, CPlus [CVar iarray; CVar !Simd.ivs]);
	     Expr_assign (CVar roarray, CPlus [CVar roarray; CVar !Simd.ovs]);
	     Expr_assign (CVar ioarray, CPlus [CVar ioarray; CVar !Simd.ovs]);
	     make_volatile_stride (CVar istride);
	     make_volatile_stride (CVar rostride);
	     make_volatile_stride (CVar iostride)
	   ],
	  Asch annot)
   ])
  in

  let tree =
    Fcn ((if !Magic.standalone then "void" else "static void"), name,
	 ([Decl (C.constrealtypep, iarray);
	   Decl (C.realtypep, roarray);
	   Decl (C.realtypep, ioarray);
	   Decl (C.stridetype, istride);
	   Decl (C.stridetype, rostride);
	   Decl (C.stridetype, iostride);
	   Decl ("INT", v);
	   Decl ("INT", "ivs");
	   Decl ("INT", "ovs")]),
	 add_constants body)

  in let desc = 
    Printf.sprintf 
      "static const kr2hc_desc desc = { %d, \"%s\", %s, &GENUS, %s, %s, %s, %s, %s };\n\n"
      n name (flops_of tree) 
      (stride_to_solverparm !uistride) 
      (stride_to_solverparm !urostride) (stride_to_solverparm !uiostride)
      "0" "0"

  and init =
    (declare_register_fcn name) ^
    "{" ^
    "  X(k" ^ kind ^ "_register)(p, " ^ name ^ ", &desc);\n" ^
    "}\n"

  in
  (unparse cvsid tree) ^ "\n" ^ (if !Magic.standalone then "" else desc ^ init)


let main () =
  begin
    parse speclist usage;
    print_string (generate (check_size ()));
  end

let _ = main()
