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
(* $Id: gen_athnotw.ml,v 1.5 2006-01-05 03:04:27 stevenj Exp $ *)

open Util
open Genutil
open C

let cvsid = "$Id: gen_athnotw.ml,v 1.5 2006-01-05 03:04:27 stevenj Exp $"

let usage = "Usage: " ^ Sys.argv.(0) ^ " -n <number>"

let mkloc n a loc = 
  array n (fun i -> 
    let klass = Unique.make () in
    let (rloc, iloc) = loc i in
    let aref = C.array_subscript a (C.SInteger 1) i in
    (Variable.make_locative (Variable.Real i) rloc klass (C.real_of aref),
     Variable.make_locative (Variable.Imag i) iloc klass (C.imag_of aref)))

let generate n =
  let a = "A"
  in

  let ns = string_of_int n
  and sign = !Genutil.sign 
  and name = !Magic.codelet_name in
  let name0 = name ^ "_0" in

  let locations = unique_array_c n in
  let input = mkloc n a locations in
  let output = Fft.dft sign n (load_array_c n input) in
  let oloc = mkloc n a locations in
  let odag = store_array_c n oloc output in
  let annot = standard_optimizer odag in

  let tree0 =
    Fcn ("static NOTW_INLINE void", name0,
	 [Decl (C.complextypep, a)],
	 Asch annot)

  in let decl t v = t ^ " " ^ v in

  let loop =
    "void " ^ name ^ "(CX *A, int v, int iv)\n" ^
    "{\n" ^
    "int i;\n" ^
    "for (i = v; i > 0; --i, A += iv) {\n" ^
    name0 ^ "(A);\n" ^
    "}\n}\n\n"

  in
  (C.unparse cvsid tree0) ^ "\n" ^ loop


let main () =
  begin
    parse speclist usage;
    print_string (generate (check_size ()));
  end

let _ = main()
