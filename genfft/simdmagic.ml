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
(* $Id: simdmagic.ml,v 1.5 2002-07-04 00:32:28 athena Exp $ *)

type memop =
  | VEC   (* load/store vector *)
  | RI    (* separate R/I *)
  | RI2   (* separate R/I, stride 2 *)
  | TR    (* transpose *)

(* SIMD magic parameters *)
let collect_load = ref false
let collect_store = ref false
let collect_twiddle = ref false
let stmode = ref VEC
let ldmode = ref VEC
let vector_length = ref 4
let transform_length = ref 0
let simd_mode = ref false

let set_mode var mode = Arg.Unit (fun () -> var := mode)

open Magic

let speclist = [
  "-simd", set_bool simd_mode, undocumented;
  "-collect-load", set_bool collect_load, undocumented;
  "-collect-store", set_bool collect_store, undocumented;
  "-collect-twiddle", set_bool collect_twiddle, undocumented;
  "-stvec", set_mode stmode VEC, undocumented;
  "-stri", set_mode stmode RI, undocumented;
  "-stri2", set_mode stmode RI2, undocumented;
  "-sttr", set_mode stmode TR, undocumented;
  "-ldvec", set_mode ldmode VEC, undocumented;
  "-ldri", set_mode ldmode RI, undocumented;
  "-ldri2", set_mode ldmode RI2, undocumented;
  "-ldtr", set_mode ldmode TR, undocumented;
  "-veclen", set_int vector_length, undocumented;
]

let f_collect_store () = 
  !collect_store || 
  match !stmode with
  | VEC -> false
  | _ -> true

let f_collect_load () = 
  !collect_load || 
  match !ldmode with
  | VEC -> false
  | _ -> true

let f_collect_twiddle () = !collect_twiddle

let f_store_transpose () =
  match !stmode with
  | TR -> true
  | _ -> false
