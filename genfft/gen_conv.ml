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
(* $Id: gen_conv.ml,v 1.8 2006-02-12 23:34:12 athena Exp $ *)

open Util
open Genutil
open C

let cvsid = "$Id: gen_conv.ml,v 1.8 2006-02-12 23:34:12 athena Exp $"

let usage = "Usage: " ^ Sys.argv.(0) ^ " -n <number>"

let generate n =
  let aa = "A"
  and bb = "B"
  and oo = "O"
  and bogus = "BOGUS"
  in

  let name = !Magic.codelet_name in

  let n2 = 2 * n - 1 in

  let inputA = 
    load_array_r n
      (locative_array_c n 
	 (C.array_subscript aa (C.SInteger 1))
	 (C.array_subscript bogus (C.SInteger 1))
	 (unique_array_c n))
  and inputB = 
    load_array_r n
      (locative_array_c n 
	 (C.array_subscript bb (C.SInteger 1))
	 (C.array_subscript bogus (C.SInteger 1))
	 (unique_array_c n))
  in
  let output = Conv.conv n inputA n inputB in

  let oloc = 
    locative_array_c n2 
      (C.array_subscript oo (C.SInteger 1))
      (C.array_subscript bogus (C.SInteger 1))
      (unique_array_c n2) in
  let odag = store_array_r n2 oloc output in
  let annot = standard_optimizer odag in

  let tree =
    Fcn ("void", name,
	 ([Decl (C.constrealtypep, aa);
	   Decl (C.constrealtypep, bb);
	   Decl (C.realtypep, oo)] 
	 ),
	 Asch annot)

  in
  (unparse cvsid tree)


let main () =
  begin
    Magic.network_transposition := false;
    Magic.inline_single := false;

    parse speclist usage;
    print_string (generate (check_size ()));
  end

let _ = main()
