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
(* $Id: simdmagic.ml,v 1.3 2002-06-30 18:37:55 athena Exp $ *)

(* SIMD magic parameters *)
let collect_load = ref false
let collect_store = ref false
let collect_twiddle = ref false
let store_transpose = ref false
let vector_length = ref 4
let transform_length = ref 0
let simd_mode = ref false
let stvec = ref true
let ldvec = ref true

open Magic

let speclist = [
  "-simd", set_bool simd_mode, undocumented;
  "-collect-load", set_bool collect_load, undocumented;
  "-collect-store", set_bool collect_store, undocumented;
  "-collect-twiddle", set_bool collect_twiddle, undocumented;
  "-store-transpose", set_bool store_transpose, undocumented;
  "-stvec", set_bool stvec, undocumented;
  "-stri", unset_bool stvec, undocumented;
  "-ldvec", set_bool ldvec, undocumented;
  "-ldri", unset_bool ldvec, undocumented;
  "-vector-length", set_int vector_length, undocumented;
]

let f_collect_store () = !collect_store || !store_transpose || (not !stvec)
let f_collect_load () = !collect_load || (not !ldvec)
let f_collect_twiddle () = !collect_twiddle
