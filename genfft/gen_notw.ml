(*
 * Copyright (c) 1997-1999 Massachusetts Institute of Technology
 * Copyright (c) 2000 Matteo Frigo
 * Copyright (c) 2000 Steven G. Johnson
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
(* $Id: gen_notw.ml,v 1.3 2002-06-07 11:07:46 athena Exp $ *)

open Util
open Genutil
open C

let cvsid = "$Id: gen_notw.ml,v 1.3 2002-06-07 11:07:46 athena Exp $"

let usage = "Usage: " ^ Sys.argv.(0) ^ " -n <number>"

let uistride = ref Stride_variable
let uostride = ref Stride_variable

let speclist = [
  "-with-istride",
  Arg.String(fun x -> uistride := arg_to_stride x),
  " specialize for given input stride";

  "-with-ostride",
  Arg.String(fun x -> uostride := arg_to_stride x),
  " specialize for given output stride"
] 

let generate n =
  let riarray = "ri"
  and iiarray = "ii"
  and roarray = "ro"
  and ioarray = "io"
  and istride = "is"
  and ostride = "os" 
  in

  let ns = string_of_int n
  and sign = !Genutil.sign 
  and name = !Magic.codelet_name in
  let name0 = name ^ "_0" in

  let vistride = either_stride (!uistride) (C.SVar istride)
  and vostride = either_stride (!uostride) (C.SVar ostride)
  in

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
    Fcn ("static INLINE_KDFT void", name0,
	 ([Decl (C.constrealtypep, riarray);
	   Decl (C.constrealtypep, iiarray);
	   Decl (C.realtypep, roarray);
	   Decl (C.realtypep, ioarray)] 
	  @ (if stride_fixed !uistride then [] 
	       else [Decl (C.stridetype, istride)])
	  @ (if stride_fixed !uostride then [] 
	       else [Decl (C.stridetype, ostride)])
	 ),
	 Asch annot)

  in let loop =
    "static void " ^ name ^
      "(const " ^ C.realtype ^ " *ri, const " ^ C.realtype ^ " *ii, "
      ^ C.realtype ^ " *ro, " ^ C.realtype ^ " *io,\n" ^ 
      C.stridetype ^ " is, " ^  C.stridetype ^ " os, " ^ 
      " uint v, int ivs, int ovs)\n" ^
    "{\n" ^
    "uint i;\n" ^
    "for (i = 0; i < v; ++i) {\n" ^
    name0 ^ "(ri + i * ivs, ii + i * ivs, ro + i * ovs, io + i * ovs" ^
       (if stride_fixed !uistride then "" else ", is") ^ 
       (if stride_fixed !uostride then "" else ", os") ^ 
            ");\n" ^
    "}\n}\n\n"

  and desc = 
    Printf.sprintf "static const kdft_desc desc = { %s, %s, %s, %s };\n"
      ns (stride_to_solverparm !uistride) (stride_to_solverparm !uostride)
      (flops_of tree0)

  and init =
    (declare_register_fcn name) ^
    "{" ^
    "  fftw_kdft_register(p, " ^ name ^ ", &desc);\n" ^
    "}\n"

  in
  (C.unparse cvsid tree0) ^ "\n" ^ loop ^ desc ^ init


let main () =
  begin
    parse speclist usage;
    print_string (generate (check_size ()));
  end

let _ = main()
