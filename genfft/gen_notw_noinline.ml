(*
 * Copyright (c) 1997-1999 Massachusetts Institute of Technology
 * Copyright (c) 2003 Matteo Frigo
 * Copyright (c) 2003 Massachusetts Institute of Technology
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
(* $Id: gen_notw_noinline.ml,v 1.1 2003-04-17 11:07:19 athena Exp $ *)

open Util
open Genutil
open C

let cvsid = "$Id: gen_notw_noinline.ml,v 1.1 2003-04-17 11:07:19 athena Exp $"

let usage = "Usage: " ^ Sys.argv.(0) ^ " -n <number>"

let uistride = ref Stride_variable
let uostride = ref Stride_variable
let uivstride = ref Stride_variable
let uovstride = ref Stride_variable

let speclist = [
  "-with-istride",
  Arg.String(fun x -> uistride := arg_to_stride x),
  " specialize for given input stride";

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
  and roarray = "ro"
  and ioarray = "io"
  and istride = "is"
  and ostride = "os" 
  in

  let sign = !Genutil.sign 
  and name = !Magic.codelet_name in
  let ename = expand_name name in
  let name0 = ename ^ "_0" in

  let vl = choose_simd "1" "VL"
  in

  let vistride = either_stride (!uistride) (C.SVar istride)
  and vostride = either_stride (!uostride) (C.SVar ostride)
  in

  let _ = Simd.ovs := stride_to_string "ovs" !uovstride in
  let _ = Simd.ivs := stride_to_string "ivs" !uivstride in

  let locations = unique_array_c n in
  let input = 
    locative_array_c n 
      (C.array_subscript riarray vistride)
      (C.array_subscript iiarray vistride)
      locations in
  let output = Fft.dft sign n (load_array_c n input) in
  let oloc = 
    locative_array_c n 
      (C.array_subscript roarray vostride)
      (C.array_subscript ioarray vostride)
      locations in
  let odag = store_array_c n oloc output in
  let annot = standard_optimizer odag in

  let tree0 =
    Fcn ("static void", name0,
	 ([Decl (C.constrealtypep, riarray);
	   Decl (C.constrealtypep, iiarray);
	   Decl (C.realtypep, roarray);
 	   Decl (C.realtypep, ioarray)] 
	  @ (if stride_fixed !uistride then [] 
	       else [Decl (C.stridetype, istride)])
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
    "static void " ^ ename ^
      "(const " ^ C.realtype ^ " *ri, const " ^ C.realtype ^ " *ii, "
      ^ C.realtype ^ " *ro, " ^ C.realtype ^ " *io,\n" ^ 
      C.stridetype ^ " is, " ^  C.stridetype ^ " os, " ^ 
      " int v, int ivs, int ovs)\n" ^
    "{\n" ^
    "int i;\n" ^
    "for (i = v; i > 0; i -= " ^ vl ^ ") {\n" ^
      name0 ^ 
        "(ri, ii, ro, io" ^
           (if stride_fixed !uistride then "" else ", is") ^ 
           (if stride_fixed !uostride then "" else ", os") ^ 
           (choose_simd ""
	      (if stride_fixed !uivstride then "" else ", ivs")) ^ 
           (choose_simd ""
	      (if stride_fixed !uovstride then "" else ", ovs")) ^ 
          ");\n" ^
      (choose_simd
	 (Printf.sprintf
	    "ri += ivs; ii += ivs; ro += %s; io += %s;\n"
	    !Simd.ovs !Simd.ovs)
	 (Printf.sprintf
	    "ri += VL * %s; ii += VL * %s; ro += VL * %s; io += VL * %s;\n"
	    !Simd.ivs !Simd.ivs !Simd.ovs !Simd.ovs)) ^
    "}\n}\n\n"

  and desc = 
    Printf.sprintf 
      "static const kdft_desc desc = { %d, \"%s\", %s, &GENUS, %s, %s, %s, %s };\n"
      n name (flops_of tree0) 
      (stride_to_solverparm !uistride) (stride_to_solverparm !uostride)
      (choose_simd "0" (stride_to_solverparm !uivstride))
      (choose_simd "0" (stride_to_solverparm !uovstride))

  and init =
    (declare_register_fcn name) ^
    "{" ^
    "  X(kdft_register)(p, " ^ ename ^ ", &desc);\n" ^
    "}\n"

  in ((unparse cvsid tree0) ^ "\n" ^ 
      loop ^ 
      desc ^
      init)

let main () =
  begin
    parse speclist usage;
    print_string (generate (check_size ()));
  end

let _ = main()
