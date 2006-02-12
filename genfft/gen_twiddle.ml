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
(* $Id: gen_twiddle.ml,v 1.24 2006-02-12 23:34:12 athena Exp $ *)

open Util
open Genutil
open C

let cvsid = "$Id: gen_twiddle.ml,v 1.24 2006-02-12 23:34:12 athena Exp $"

type ditdif = DIT | DIF
let ditdif = ref DIT
let usage = "Usage: " ^ Sys.argv.(0) ^ " -n <number> [ -dit | -dif ]"

let uiostride = ref Stride_variable
let udist = ref Stride_variable

let speclist = [
  "-dit",
  Arg.Unit(fun () -> ditdif := DIT),
  " generate a DIT codelet";

  "-dif",
  Arg.Unit(fun () -> ditdif := DIF),
  " generate a DIF codelet";

  "-with-iostride",
  Arg.String(fun x -> uiostride := arg_to_stride x),
  " specialize for given i/o stride";

  "-with-dist",
  Arg.String(fun x -> udist := arg_to_stride x),
  " specialize for given dist"
]

let generate n =
  let rioarray = "ri"
  and iioarray = "ii"
  and iostride = "ios"
  and twarray = "W"
  and i = "i" 
  and m = "m"
  and dist = "dist" in

  let sign = !Genutil.sign 
  and name = !Magic.codelet_name 
  and byvl x = choose_simd x (ctimes (CVar "(2 * VL)", x)) in
  let ename = expand_name name in

  let (bytwiddle, num_twiddles, twdesc) = Twiddle.twiddle_policy false in
  let nt = num_twiddles n in

  let byw = bytwiddle n sign (twiddle_array nt twarray) in

  let viostride = either_stride (!uiostride) (C.SVar iostride) in
  let _ = Simd.ovs := stride_to_string "dist" !udist in
  let _ = Simd.ivs := stride_to_string "dist" !udist in

  let locations = unique_array_c n in
  let iloc = 
    locative_array_c n 
      (C.array_subscript rioarray viostride)
      (C.array_subscript iioarray viostride)
      locations
  and oloc = 
    locative_array_c n 
      (C.array_subscript rioarray viostride)
      (C.array_subscript iioarray viostride)
      locations 
  in
  let liloc = load_array_c n iloc in
  let output =
    match !ditdif with
    | DIT -> array n (Fft.dft sign n (byw liloc))
    | DIF -> array n (byw (Fft.dft sign n liloc))
  in
  let odag = store_array_c n oloc output in
  let annot = standard_optimizer odag in

  let body = Block (
    [Decl ("INT", i)],
    [For (Expr_assign (CVar i, CVar m),
	  Binop (" > ", CVar i, Integer 0),
	  list_to_comma 
	    [Expr_assign (CVar i, CPlus [CVar i; CUminus (byvl (Integer 1))]);
	     Expr_assign (CVar rioarray, CPlus [CVar rioarray; 
						byvl (CVar !Simd.ivs)]);
	     Expr_assign (CVar iioarray, CPlus [CVar iioarray; 
						byvl (CVar !Simd.ivs)]);
	     Expr_assign (CVar twarray, CPlus [CVar twarray; 
					       byvl (Integer nt)]);
	     make_volatile_stride (CVar iostride)
	   ],
	  Asch annot);
     Return (CVar twarray)
   ])
  in

  let tree = 
    Fcn (("static " ^ C.constrealtypep), ename,
	 [Decl (C.realtypep, rioarray);
	  Decl (C.realtypep, iioarray);
	  Decl (C.constrealtypep, twarray);
	  Decl (C.stridetype, iostride);
	  Decl ("INT", m);
	  Decl ("INT", dist)],
         add_constants body)
  in
  let twinstr = 
    Printf.sprintf "static const tw_instr twinstr[] = %s;\n\n" 
      (twinstr_to_string "(2 * VL)" (twdesc n))
  and desc = 
    Printf.sprintf
      "static const ct_desc desc = {%d, \"%s\", twinstr, &GENUS, %s, %s, %s, %s};\n\n"
      n name (flops_of tree) 
      (stride_to_solverparm !uiostride) "0"
      (stride_to_solverparm !udist) 
  and register = 
    match !ditdif with
    | DIT -> "X(kdft_dit_register)"
    | DIF -> "X(kdft_dif_register)"

  in
  let init =
    "\n" ^
    twinstr ^ 
    desc ^
    (declare_register_fcn name) ^
    (Printf.sprintf "{\n%s(p, %s, &desc);\n}" register ename)
  in

  (unparse cvsid tree) ^ "\n" ^ init


let main () =
  begin 
    parse (speclist @ Twiddle.speclist) usage;
    print_string (generate (check_size ()));
  end

let _ = main()
