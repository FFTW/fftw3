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
(* $Id: gen_notw.ml,v 1.30 2006-02-12 23:34:12 athena Exp $ *)

open Util
open Genutil
open C

let cvsid = "$Id: gen_notw.ml,v 1.30 2006-02-12 23:34:12 athena Exp $"

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

let nonstandard_optimizer list_of_buddy_stores dag =
  let sched = standard_scheduler dag in
  let annot = Annotate.annotate list_of_buddy_stores sched in
  let _ = dump_asched annot in
  annot

let generate n =
  let riarray = "ri"
  and iiarray = "ii"
  and roarray = "ro"
  and ioarray = "io"
  and istride = "is"
  and ostride = "os" 
  and i = "i" 
  and v = "v"
  in

  let sign = !Genutil.sign 
  and name = !Magic.codelet_name
  and byvl x = choose_simd x (ctimes (CVar "(2 * VL)", x))  in
  let ename = expand_name name in

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
  let list_of_buddy_stores =
    let k = !Simdmagic.store_multiple in
    if (k > 1) then
      if (n mod k == 0) then
	List.append
	  (List.map 
	     (fun i -> List.map (fun j -> (fst (oloc (k * i + j)))) (iota k))
	     (iota (n / k)))
	  (List.map 
	     (fun i -> List.map (fun j -> (snd (oloc (k * i + j)))) (iota k))
	     (iota (n / k)))
      else failwith "invalid n for -store-multiple"
    else []
  in

  let odag = store_array_c n oloc output in
  let annot = nonstandard_optimizer list_of_buddy_stores odag in

  let body = Block (
    [Decl ("INT", i)],
    [For (Expr_assign (CVar i, CVar v),
	  Binop (" > ", CVar i, Integer 0),
	  list_to_comma 
	    [Expr_assign (CVar i, CPlus [CVar i; CUminus (byvl (Integer 1))]);
	     Expr_assign (CVar riarray, CPlus [CVar riarray; 
					       byvl (CVar !Simd.ivs)]);
	     Expr_assign (CVar iiarray, CPlus [CVar iiarray; 
					       byvl (CVar !Simd.ivs)]);
	     Expr_assign (CVar roarray, CPlus [CVar roarray; 
					       byvl (CVar !Simd.ovs)]);
	     Expr_assign (CVar ioarray, CPlus [CVar ioarray; 
					       byvl (CVar !Simd.ovs)]);
	     make_volatile_stride (CVar istride);
	     make_volatile_stride (CVar ostride)
	   ],
	  Asch annot)
   ])
  in

  let tree =
    Fcn ((if !Magic.standalone then "void" else "static void"), ename,
	 ([Decl (C.constrealtypep, riarray);
	   Decl (C.constrealtypep, iiarray);
	   Decl (C.realtypep, roarray);
 	   Decl (C.realtypep, ioarray);
	   Decl (C.stridetype, istride);
	   Decl (C.stridetype, ostride);
	   Decl ("INT", v);
	   Decl ("INT", "ivs");
	   Decl ("INT", "ovs")]),
	 add_constants body)

  in let desc = 
    Printf.sprintf 
      "static const kdft_desc desc = { %d, %s, %s, &GENUS, %s, %s, %s, %s };\n"
      n (stringify name) (flops_of tree) 
      (stride_to_solverparm !uistride) (stride_to_solverparm !uostride)
      (choose_simd "0" (stride_to_solverparm !uivstride))
      (choose_simd "0" (stride_to_solverparm !uovstride))

  and init =
    (declare_register_fcn name) ^
    "{" ^
    "  X(kdft_register)(p, " ^ ename ^ ", &desc);\n" ^
    "}\n"

  in ((unparse cvsid tree) ^ "\n" ^ 
      (if !Magic.standalone then "" else desc ^ init))

let main () =
  begin
    parse speclist usage;
    print_string (generate (check_size ()));
  end

let _ = main()
