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
(* $Id: gen_twidsq.ml,v 1.19 2006-02-12 23:34:12 athena Exp $ *)

open Util
open Genutil
open C

let cvsid = "$Id: gen_twidsq.ml,v 1.19 2006-02-12 23:34:12 athena Exp $"
type ditdif = DIT | DIF
let ditdif = ref DIT

let usage = "Usage: " ^ Sys.argv.(0) ^ " -n <number> [ -dit | -dif ]"

let reload_twiddle = ref false

let uistride = ref Stride_variable
let uvstride = ref Stride_variable
let udist = ref Stride_variable

let speclist = [
  "-dit",
  Arg.Unit(fun () -> ditdif := DIT),
  " generate a DIT codelet";

  "-dif",
  Arg.Unit(fun () -> ditdif := DIF),
  " generate a DIF codelet";

  "-reload-twiddle",
  Arg.Unit(fun () -> reload_twiddle := true),
  " do not collect common twiddle factors";

  "-with-istride",
  Arg.String(fun x -> uistride := arg_to_stride x),
  " specialize for given input stride";

  "-with-vstride",
  Arg.String(fun x -> uvstride := arg_to_stride x),
  " specialize for given vector stride";

  "-with-dist",
  Arg.String(fun x -> udist := arg_to_stride x),
  " specialize for given dist"
]

let generate n =
  let rioarray = "rio"
  and iioarray = "iio"
  and istride = "is"
  and vstride = "vs"
  and twarray = "W" 
  and i = "i" 
  and m = "m"
  and dist = "dist" in

  let sign = !Genutil.sign 
  and name = !Magic.codelet_name in

  let (bytwiddle, num_twiddles, twdesc) = Twiddle.twiddle_policy false in
  let nt = num_twiddles n in

  let svstride = either_stride (!uvstride) (C.SVar vstride)
  and sistride = either_stride (!uistride) (C.SVar istride) in

  let byw =
    if !reload_twiddle then
      array n (fun v -> bytwiddle n sign (twiddle_array nt twarray))
    else
      let a = bytwiddle n sign (twiddle_array nt twarray)
      in fun v -> a
  in

  let locations = unique_v_array_c n n in

  let ioi = 
    locative_v_array_c n n 
      (C.varray_subscript rioarray svstride sistride) 
      (C.varray_subscript iioarray svstride sistride) 
      locations 
  and ioo = 
    locative_v_array_c n n 
      (C.varray_subscript rioarray svstride sistride) 
      (C.varray_subscript iioarray svstride sistride) 
      locations 
  in

  let lioi = load_v_array_c n n ioi in
  let output =
    match !ditdif with
    | DIT -> array n (fun v -> Fft.dft sign n (byw v (lioi v)))
    | DIF -> array n (fun v -> byw v (Fft.dft sign n (lioi v)))
  in

  let odag = store_v_array_c n n ioo (transpose output) in
  let annot = standard_optimizer odag in

  let body = Block (
    [Decl ("INT", i)],
    [For (Expr_assign (CVar i, CVar m),
	  Binop (" > ", CVar i, Integer 0),
	  list_to_comma 
	    [Expr_assign (CVar i, CPlus [CVar i; CUminus (Integer 1)]);
	     Expr_assign (CVar rioarray, CPlus [CVar rioarray; CVar dist]);
	     Expr_assign (CVar iioarray, CPlus [CVar iioarray; CVar dist]);
	     Expr_assign (CVar twarray, CPlus [CVar twarray; Integer nt]);
	     make_volatile_stride (CVar istride);
	     make_volatile_stride (CVar vstride)
	   ],
	  Asch annot);
     Return (CVar twarray)
   ]) in

  let tree = 
    Fcn (("static " ^ C.constrealtypep), name,
	 [Decl (C.realtypep, rioarray);
	  Decl (C.realtypep, iioarray);
	  Decl (C.constrealtypep, twarray);
	  Decl (C.stridetype, istride);
	  Decl (C.stridetype, vstride);
	  Decl ("INT", m);
	  Decl ("INT", dist)],
         add_constants body)
  in
  let twinstr = 
    Printf.sprintf "static const tw_instr twinstr[] = %s;\n\n" 
      (Twiddle.twinstr_to_c_string (twdesc n))

  and desc = 
    Printf.sprintf
      "static const ct_desc desc = {%d, \"%s\", twinstr, &GENUS, %s, %s, %s, %s};\n\n"
      n name (flops_of tree) 
      (stride_to_solverparm !uistride) (stride_to_solverparm !uvstride)
      (stride_to_solverparm !udist) 

  and register = 
    match !ditdif with
    | DIT -> "X(kdft_ditsq_register)"
    | DIF -> "X(kdft_difsq_register)"
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
