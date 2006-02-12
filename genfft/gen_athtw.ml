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
(* $Id: gen_athtw.ml,v 1.7 2006-02-12 23:34:12 athena Exp $ *)

open Util
open Genutil
open C

let cvsid = "$Id: gen_athtw.ml,v 1.7 2006-02-12 23:34:12 athena Exp $"

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

let stride = "STRIDE"

let mkloc n a loc = 
  array n (fun i -> 
    let klass = Unique.make () in
    let (rloc, iloc) = loc i in
    let aref = C.array_subscript a (C.SVar stride) i in
    (Variable.make_locative (Variable.Real i) rloc klass (C.real_of aref),
     Variable.make_locative (Variable.Imag i) iloc klass (C.imag_of aref)))

let generate n =
  let a = "a"
  and twarray = "W"
  and i = "i" in

  let ns = string_of_int n 
  and sign = !Genutil.sign 
  and name = !Magic.codelet_name in

  let (bytwiddle, num_twiddles, twdesc) = Twiddle.twiddle_policy false in
  let nt = num_twiddles n in

  let byw = bytwiddle n sign (twiddle_array nt twarray) in

  let locations = unique_array_c n in
  let iloc = mkloc n a locations in
  let oloc = mkloc n a locations in

  let liloc = load_array_c n iloc in
  let output =
    match !ditdif with
    | DIT -> array n (Fft.dft sign n (byw liloc))
    | DIF -> array n (byw (Fft.dft sign n liloc))
  in
  let odag = store_array_c n oloc output in
  let annot = standard_optimizer odag in

  let body = Block (
    [Decl ("int", i)],
    [For (Expr_assign (CVar i, CVar stride),
	  Binop (" > ", CVar i, Integer 0),
	  list_to_comma 
	    [Expr_assign (CVar i, CPlus [CVar i; CUminus (Integer 1)]);
	     Expr_assign (CVar a, CPlus [CVar a; Integer 1]);
	     Expr_assign (CVar twarray, CPlus [CVar twarray; Integer nt])],
	  Asch annot)] )
   
  in

  let tree = 
    Fcn ((if !Magic.standalone then "void" else "static void"), name,
	 [Decl (C.complextypep, a);
	  Decl (C.constrealtypep, twarray)],
         body)
  in

  (C.unparse cvsid tree) ^ "\n"


let main () =
  begin 
    parse (speclist @ Twiddle.speclist) usage;
    print_string (generate (check_size ()));
  end

let _ = main()
