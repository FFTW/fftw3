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
(* $Id: gen_trig.ml,v 1.7 2003-03-15 20:29:42 stevenj Exp $ *)

(* generation of trigonometric transforms *)

open Util
open Genutil
open C

let cvsid = "$Id: gen_trig.ml,v 1.7 2003-03-15 20:29:42 stevenj Exp $"

let usage = "Usage: " ^ Sys.argv.(0) ^ " -n <number>"

type mode =
  | RDFT
  | HDFT
  | DHT
  | DCTI
  | DCTII
  | DCTIII
  | DCTIV
  | DSTI
  | DSTII
  | DSTIII
  | DSTIV
  | NONE

let mode = ref NONE

let speclist = [
  "-rdft",
  Arg.Unit(fun () -> mode := RDFT),
  " generate a real DFT codelet";

  "-hdft",
  Arg.Unit(fun () -> mode := HDFT),
  " generate a Hermitian DFT codelet";

  "-dht",
  Arg.Unit(fun () -> mode := DHT),
  " generate a DHT codelet";

  "-dct-I",
  Arg.Unit(fun () -> mode := DCTI),
  " generate a DCT-I codelet";

  "-dct-II",
  Arg.Unit(fun () -> mode := DCTII),
  " generate a DCT-II codelet";

  "-dct-III",
  Arg.Unit(fun () -> mode := DCTIII),
  " generate a DCT-III codelet";

  "-dct-IV",
  Arg.Unit(fun () -> mode := DCTIV),
  " generate a DCT-IV codelet";

  "-dst-I",
  Arg.Unit(fun () -> mode := DSTI),
  " generate a DST-I codelet";

  "-dst-II",
  Arg.Unit(fun () -> mode := DSTII),
  " generate a DST-II codelet";

  "-dst-III",
  Arg.Unit(fun () -> mode := DSTIII),
  " generate a DST-III codelet";

  "-dst-IV",
  Arg.Unit(fun () -> mode := DSTIV),
  " generate a DST-IV codelet";
]

let generate n mode =
  let riarray = "input"
  and iiarray = "BUG"
  and roarray = "output"
  and ioarray = "BUG"
  and istride = "istride"
  and ostride = "ostride" in

  let ns = string_of_int n
  and name = !Magic.codelet_name in

  let (transform, load_input, store_output) = match mode with
  | RDFT -> Trig.rdft (-1), load_array_r, store_array_hc
  | HDFT -> Trig.hdft (-1), load_array_c, store_array_r  (* TODO *)
  | DHT -> Trig.dht 1, load_array_r, store_array_r
  | DCTI -> Trig.dctI, load_array_r, store_array_r
  | DCTII -> Trig.dctII, load_array_r, store_array_r
  | DCTIII -> Trig.dctIII, load_array_r, store_array_r
  | DCTIV -> Trig.dctIV, load_array_r, store_array_r
  | DSTI -> Trig.dstI, load_array_r, store_array_r
  | DSTII -> Trig.dstII, load_array_r, store_array_r
  | DSTIII -> Trig.dstIII, load_array_r, store_array_r
  | DSTIV -> Trig.dstIV, load_array_r, store_array_r
  | _ -> failwith "must specify transform kind"
  in
    
  let locations = unique_array_c n in
  let input = 
    locative_array_c n 
      (C.array_subscript riarray (C.SVar istride))
      (C.array_subscript iiarray (C.SVar istride))
      locations in
  let output = transform n (load_array_c n input) in
  let oloc = 
    locative_array_c n 
      (C.array_subscript roarray (C.SVar ostride))
      (C.array_subscript ioarray (C.SVar ostride))
      locations in
  let odag = store_output n oloc output in
  let annot = standard_optimizer odag in

  let tree = 
    Fcn ("static void", name,
	 [Decl (C.constrealtypep, riarray);
	  Decl (C.realtypep, roarray);
	  Decl (C.stridetype, istride);
	  Decl (C.stridetype, ostride)],
	 add_constants (Asch annot))
  and init =
    (declare_register_fcn name) ^
    "{" ^
    "  fftw_codelet_notw_register(p, " ^ name ^ ", " ^ ns ^ ");\n" ^ 
    "}\n"

  in
  (unparse cvsid tree) ^ "\n" (* ^ init *)


let main () =
  begin
    parse speclist usage;
    print_string (generate (check_size ()) !mode);
  end

let _ = main()
