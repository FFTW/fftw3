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
(* $Id: gen_hc2hc.ml,v 1.16 2006-02-12 23:34:12 athena Exp $ *)

open Util
open Genutil
open C

let cvsid = "$Id: gen_hc2hc.ml,v 1.16 2006-02-12 23:34:12 athena Exp $"

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

let rioarray = "rio" 
and iioarray = "iio" 

let genone sign n transform load store viostride =
  let locations = unique_array_c n in
  let input = 
    locative_array_c n 
      (C.array_subscript rioarray viostride)
      (C.array_subscript iioarray (SNeg viostride))
      locations in
  let output = transform sign n (load n input) in
  let ioloc = 
    locative_array_c n 
      (C.array_subscript rioarray viostride)
      (C.array_subscript iioarray (SNeg viostride))
      locations in
  let odag = store n ioloc output in
  let annot = standard_optimizer odag 
  in annot

let byi = Complex.times Complex.i
let byui = Complex.times (Complex.uminus Complex.i)

let sym1 n f i = 
  Complex.plus [Complex.real (f i); byi (Complex.imag (f (n - 1 - i)))]

let sym2 n f i = if (i < n - i) then f i else byi (f i)
let sym2i n f i = if (i < n - i) then f i else byui (f i)

let generate n =
  let iostride = "ios"
  and twarray = "W"
  and i = "i" 
  and m = "m"
  and dist = "dist" in

  let sign = !Genutil.sign 
  and name = !Magic.codelet_name 
  and byvl x = choose_simd x (ctimes (CVar "VL", x)) in

  let (bytwiddle, num_twiddles, twdesc) = Twiddle.twiddle_policy false in
  let nt = num_twiddles n in

  let byw = bytwiddle n sign (twiddle_array nt twarray) in

  let viostride = either_stride (!uiostride) (C.SVar iostride) in
  let _ = Simd.ovs := stride_to_string "dist" !udist in
  let _ = Simd.ivs := stride_to_string "dist" !udist in

  let asch = 
    match !ditdif with
    | DIT -> 
	genone sign n 
	  (fun sign n input -> sym2 n (Fft.dft sign n (byw (sym1 n input))))
	  load_array_c store_array_c viostride
    | DIF -> 
	genone sign n 
	  (fun sign n input -> sym1 n (byw (Fft.dft sign n (sym2i n input))))
	  load_array_c store_array_c viostride
  in

  let vdist = CVar !Simd.ivs 
  and vrioarray = CVar rioarray
  and viioarray = CVar iioarray
  and vi = CVar i  and vm = CVar m 
  in
  let body = Block (
    [Decl ("INT", i)],
    [For (Expr_assign (vi, (CPlus [vm; CUminus (Integer 2)])),
	  Binop (" > ", vi, Integer 0),
	  list_to_comma 
	    [Expr_assign (vi, CPlus [vi; CUminus (byvl (Integer 2))]);
	     Expr_assign (vrioarray, CPlus [vrioarray; byvl vdist]);
	     Expr_assign (viioarray, 
			  CPlus [viioarray; CUminus (byvl vdist)]);
	     Expr_assign (CVar twarray, CPlus [CVar twarray; 
					       byvl (Integer nt)]);
	     make_volatile_stride (CVar iostride)
	   ],
	  Asch asch);
     Return (CVar twarray)]
    )
  in

  let tree = 
    Fcn ("static const R *", name,
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
      (twinstr_to_string "VL" (twdesc n))
  and desc = 
    Printf.sprintf
      "static const hc2hc_desc desc = {%d, \"%s\", twinstr, &GENUS, %s, %s, %s, %s};\n\n"
      n name (flops_of tree)
      (stride_to_solverparm !uiostride) "0"
      (stride_to_solverparm !udist) 
  and register = "X(khc2hc_register)"

  in
  let init =
    "\n" ^
    twinstr ^ 
    desc ^
    (declare_register_fcn name) ^
    (Printf.sprintf "{\n%s(p, %s, &desc);\n}" register name)
  in

  (unparse cvsid tree) ^ "\n" ^ init


let main () =
  begin 
    parse (speclist @ Twiddle.speclist) usage;
    print_string (generate (check_size ()));
  end

let _ = main()
