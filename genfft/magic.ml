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
(* $Id: magic.ml,v 1.15 2003-07-09 21:39:16 athena Exp $ *)

(* magic parameters *)
let verbose = ref false
let vneg = ref false
let karatsuba_min = ref 15
let karatsuba_variant = ref 2
let circular_min = ref 64
let rader_min = ref 13
let rader_list = ref [5]
let alternate_convolution = ref 17
let inline_single = ref true
let inline_loads = ref false
let inline_loads_constants = ref false
let inline_constants = ref true
let trivial_stores = ref false
let locations_are_special = ref false
let wsquare = ref false
let strength_reduce_mul = ref false
let number_of_variables = ref 4
let codelet_name = ref "unnamed"
let randomized_cse = ref true
let dif_split_radix = ref false
let enable_fma = ref false
let deep_collect_depth = ref 1
let schedule_type = ref 0
let compact = ref false
let dag_dump_file = ref ""
let alist_dump_file = ref ""
let asched_dump_file = ref ""
let lisp_syntax = ref false
let network_transposition = ref true
let inklude = ref ""
let generic_arith = ref false
let list_schedule_threshold = ref 0
let scheduler_hack = ref false

(* command-line parser for magic parameters *)
let undocumented = " Undocumented voodoo parameter"

let set_bool var = Arg.Unit (fun () -> var := true)
let unset_bool var = Arg.Unit (fun () -> var := false)
let set_int var = Arg.Int(fun i -> var := i)
let set_string var = Arg.String(fun s -> var := s)

let speclist = [
  "-name", set_string codelet_name, " set codelet name";
  "-include", set_string inklude, undocumented;

  "-verbose", set_bool verbose, " Enable verbose logging messages to stderr";

  "-rader-min", set_int rader_min,
  "<n> : Use Rader's algorithm for prime sizes >= <n>";

  "-karatsuba-min", set_int karatsuba_min, undocumented;
  "-karatsuba-variant", set_int karatsuba_variant, undocumented;
  "-circular-min", set_int circular_min, undocumented;

  "-compact", set_bool compact, 
  " Mangle variable names to reduce size of source code";
  "-no-compact", unset_bool compact, 
  " Disable -compact";

  "-dump-dag", set_string dag_dump_file, undocumented;
  "-dump-alist", set_string alist_dump_file, undocumented;
  "-dump-asched", set_string asched_dump_file, undocumented;
  "-lisp-syntax", set_bool lisp_syntax, undocumented;

  "-alternate-convolution", set_int alternate_convolution, undocumented;
  "-deep-collect-depth", set_int deep_collect_depth, undocumented;
  "-schedule-type", set_int schedule_type, undocumented;
  "-list-schedule-threshold", set_int list_schedule_threshold, undocumented;

  "-dif-split-radix", set_bool dif_split_radix, undocumented;
  "-dit-split-radix", unset_bool dif_split_radix, undocumented;

  "-generic-arith", set_bool generic_arith, undocumented;
  "-no-generic-arith", unset_bool generic_arith, undocumented;

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

  "-locations-are-special", set_bool locations_are_special, undocumented;
  "-no-locations-are-special", unset_bool locations_are_special, undocumented;

  "-randomized-cse", set_bool randomized_cse, undocumented;
  "-no-randomized-cse", unset_bool randomized_cse, undocumented;

  "-network-transposition", set_bool network_transposition, undocumented;
  "-no-network-transposition", unset_bool network_transposition, undocumented;

  "-scheduler-hack", set_bool scheduler_hack, undocumented;
  "-no-scheduler-hack", unset_bool scheduler_hack, undocumented;

  "-vneg", set_bool vneg, undocumented;
  "-fma", set_bool enable_fma, undocumented;
  "-no-fma", unset_bool enable_fma, undocumented;

  "-variables", set_int number_of_variables, undocumented;

  "-strength-reduce-mul", set_bool strength_reduce_mul, undocumented;
  "-no-strength-reduce-mul", unset_bool strength_reduce_mul, undocumented;

  "-wsquare", set_bool wsquare, undocumented;
  "-no-wsquare", unset_bool wsquare, undocumented;
] 
   

