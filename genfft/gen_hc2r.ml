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
(* $Id: gen_hc2r.ml,v 1.6 2002-07-15 20:46:36 athena Exp $ *)

open Util
open Genutil
open C

let cvsid = "$Id: gen_hc2r.ml,v 1.6 2002-07-15 20:46:36 athena Exp $"

let usage = "Usage: " ^ Sys.argv.(0) ^ " -n <number>"

let uristride = ref Stride_variable
let uiistride = ref Stride_variable
let uostride = ref Stride_variable
let uivstride = ref Stride_variable
let uovstride = ref Stride_variable

let speclist = [
  "-with-ristride",
  Arg.String(fun x -> uristride := arg_to_stride x),
  " specialize for given real input stride";

  "-with-iistride",
  Arg.String(fun x -> uiistride := arg_to_stride x),
  " specialize for given imaginary input stride";

  "-with-ostride",
  Arg.String(fun x -> uostride := arg_to_stride x),
  " specialize for given output stride";

  "-with-ivstride",
  Arg.String(fun x -> uivstride := arg_to_stride x),
  " specialize for given input vector stride";

  "-with-ovstride",
  Arg.String(fun x -> uovstride := arg_to_stride x),
  " specialize for given output vector stride"
] 


let generate n =
  let riarray = "ri"
  and iiarray = "ii"
  and oarray = "O"
  and ristride = "ris"
  and iistride = "iis"
  and ostride = "os" 
  in

  let ns = string_of_int n
  and sign = !Genutil.sign 
  and name = !Magic.codelet_name in
  let name0 = name ^ "_0" in

  let vristride = either_stride (!uristride) (C.SVar ristride)
  and viistride = either_stride (!uiistride) (C.SVar iistride)
  and vostride = either_stride (!uostride) (C.SVar ostride)
  in

  let _ = Simd.ovs := stride_to_string "ovs" !uovstride in
  let _ = Simd.ivs := stride_to_string "ivs" !uivstride in

  let locations = unique_array_c n in
  let input = 
    locative_array_c n 
      (C.array_subscript riarray vristride)
      (C.array_subscript iiarray viistride)
      locations in
  let output = Trig.hdft sign n (load_array_hc n input) in
  let oloc = 
    locative_array_c n 
      (C.array_subscript oarray vostride)
      (C.array_subscript "BUG" vostride)
      locations in
  let odag = store_array_r n oloc output in
  let annot = standard_optimizer odag in

  let tree0 =
    Fcn ("static void", name0,
	 ([Decl (C.constrealtypep, riarray);
	   Decl (C.constrealtypep, iiarray);
	   Decl (C.realtypep, oarray)]
	  @ (if stride_fixed !uristride then [] 
               else [Decl (C.stridetype, ristride)])
	  @ (if stride_fixed !uiistride then [] 
               else [Decl (C.stridetype, iistride)])
	  @ (if stride_fixed !uostride then [] 
	       else [Decl (C.stridetype, ostride)])
	  @ (choose_simd []
	       (if stride_fixed !uivstride then [] else 
	       [Decl ("int", !Simd.ivs)]))
	  @ (choose_simd []
	       (if stride_fixed !uovstride then [] else 
	       [Decl ("int", !Simd.ovs)]))
	 ),
	 add_constants (Asch annot))

  in let loop =
    "static void " ^ name ^ "(" ^ 
    "const " ^ C.realtype ^ " *ri, " ^ 
    "const " ^ C.realtype ^ " *ii, " ^ 
    C.realtype ^ " *O, " ^
    C.stridetype ^ " ris, " ^
    C.stridetype ^ " iis, " ^
    C.stridetype ^ " os, " ^ 
      " uint v, int ivs, int ovs)\n" ^
    "{\n" ^
    "uint i;\n" ^
    "for (i = 0; i < v; ++i) {\n" ^
      name0 ^ "(ri, ii, O" ^
       (if stride_fixed !uristride then "" else ", ris") ^ 
       (if stride_fixed !uiistride then "" else ", iis") ^ 
       (if stride_fixed !uostride then "" else ", os") ^ 
       (choose_simd ""
	  (if stride_fixed !uivstride then "" else ", ivs")) ^ 
       (choose_simd ""
	  (if stride_fixed !uovstride then "" else ", ovs")) ^ 
    ");\n" ^
    "ri += ivs; ii += ivs; O += ovs;\n" ^
    "}\n}\n\n"

  and desc = 
    Printf.sprintf 
      "static const khc2r_desc desc = { %d, \"%s\", %s, &GENUS, %s, %s, %s, %s, %s };\n"
      n name (flops_of tree0) 
      (stride_to_solverparm !uristride) (stride_to_solverparm !uiistride)
      (stride_to_solverparm !uostride)
      (choose_simd "0" (stride_to_solverparm !uivstride))
      (choose_simd "0" (stride_to_solverparm !uovstride))

  and init =
    (declare_register_fcn name) ^
    "{" ^
    "  X(khc2r_register)(p, " ^ name ^ ", &desc);\n" ^
    "}\n"

  in
  (unparse cvsid tree0) ^ "\n" ^ loop ^ desc ^ init


let main () =
  begin
    parse speclist usage;
    print_string (generate (check_size ()));
  end

let _ = main()
