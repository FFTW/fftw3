(*
 * Copyright (c) 1997-1999 Massachusetts Institute of Technology
 * Copyright (c) 2000-2001 Stefan Kral
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

(* magic parameters *)

let window = ref 5

let number_of_variables = ref 4

let use_wsquare = ref false

let inline_single = ref true

type twiddle_policy =
    TWIDDLE_LOAD_ALL
  | TWIDDLE_ITER
  | TWIDDLE_LOAD_ODD
  | TWIDDLE_SQUARE1
  | TWIDDLE_SQUARE2
  | TWIDDLE_SQUARE3

let twiddle_policy = ref TWIDDLE_LOAD_ALL

let inline_konstants = ref false
let inline_loads = ref false
let loopo = ref false

let rader_min = ref 13
let rader_list = ref [5]

let alternate_convolution = ref 17

let times_3_3 = ref false

let enable_fma = ref false
let enable_fma_expansion = ref false

let collect_common_twiddle = ref true
let collect_common_inputs = ref true

let verbose = ref false

let name = ref "UNNAMED"

(****************************************************************************
 * K7/FFTW-GEL SPECIFIC VALUES 						    *
 ****************************************************************************)

(* search depth limit used when translating 'imull' to a sequence of 'leal's *)
let imul_to_lea_limit = ref 3

(* insert debugging messages (as comments) into the code? *)
let do_debug_output = ref false

(* # of max. vectorization steps per vectorization-grade *)
let vectsteps_limit = ref 100000

(* target processor *)
type amd_processor =
  | AMD_K6
  | AMD_K7

let target_processor = ref AMD_K7

