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

let wsquare = ref false

let inline_single = ref true

let inline_loads = ref false
let inline_loads_constants = ref false
let inline_constants = ref true
let trivial_stores = ref false
let loopo = ref false

let rader_min = ref 13
let rader_list = ref [5]

let alternate_convolution = ref 17

let enable_fma = ref false
let enable_fma_expansion = ref false

let collect_common_twiddle = ref true
let collect_common_inputs = ref true
let strength_reduce_mul = ref false
let verbose = ref false
let randomized_cse = ref true

let codelet_name = ref "unnamed"
let dif_split_radix = ref false
let deep_collect_depth = ref 1
let network_transposition = ref true

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

(* command-line parsing *)
let set_bool var = Arg.Unit (fun () -> var := true)
let unset_bool var = Arg.Unit (fun () -> var := false)
let set_int var = Arg.Int(fun i -> var := i)
let set_string var = Arg.String(fun s -> var := s)

let undocumented = " Undocumented voodoo parameter"

let speclist =  [
  "-name", set_string codelet_name, " set codelet name";

  "-verbose", set_bool verbose, " Enable verbose logging messages to stderr";

  "-rader-min", set_int rader_min,
  "<n> : Use Rader's algorithm for prime sizes >= <n>";

  "-alternate-convolution", set_int alternate_convolution, undocumented;
  "-deep-collect-depth", set_int deep_collect_depth, undocumented;

  "-dif-split-radix", set_bool dif_split_radix, undocumented;
  "-dit-split-radix", unset_bool dif_split_radix, undocumented;

  "-inline-single", set_bool inline_single, undocumented;
  "-no-inline-single", unset_bool inline_single, undocumented;

  "-inline-loads", set_bool inline_loads, undocumented;
  "-no-inline-loads", unset_bool inline_loads, undocumented;

  "-inline-loads-constants", set_bool inline_loads_constants, undocumented;
  "-no-inline-loads-constants",
     unset_bool inline_loads_constants, undocumented;

  "-inline-constants", set_bool inline_constants, undocumented;
  "-no-inline-constants", unset_bool inline_constants, undocumented;

  "-trivial-stores", set_bool trivial_stores, undocumented;
  "-no-trivial-stores", unset_bool trivial_stores, undocumented;

  "-randomized-cse", set_bool randomized_cse, undocumented;
  "-no-randomized-cse", unset_bool randomized_cse, undocumented;

  "-network-transposition", set_bool network_transposition, undocumented;
  "-no-network-transposition", unset_bool network_transposition, undocumented;

  "-fma", set_bool enable_fma, undocumented;
  "-no-fma", unset_bool enable_fma, undocumented;

  "-variables", set_int number_of_variables, undocumented;

  "-strength-reduce-mul", set_bool strength_reduce_mul, undocumented;
  "-no-strength-reduce-mul", unset_bool strength_reduce_mul, undocumented;

  "-magic-vectsteps-limit", set_int vectsteps_limit, undocumented;

  "-magic-target-processor-k6",
    Arg.Unit(fun () -> target_processor := AMD_K6),
    " Produce code to run on an AMD K6-II+ (K6-III)";
]
