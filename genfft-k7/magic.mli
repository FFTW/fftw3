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


val window : int ref
val number_of_variables : int ref
val use_wsquare : bool ref
val inline_single : bool ref
type twiddle_policy =
    TWIDDLE_LOAD_ALL
  | TWIDDLE_ITER
  | TWIDDLE_LOAD_ODD
  | TWIDDLE_SQUARE1
  | TWIDDLE_SQUARE2
  | TWIDDLE_SQUARE3
val twiddle_policy : twiddle_policy ref
val inline_konstants : bool ref
val inline_loads : bool ref
val loopo : bool ref
val rader_min : int ref
val rader_list : int list ref
val alternate_convolution : int ref
val times_3_3 : bool ref
val enable_fma : bool ref
val enable_fma_expansion : bool ref
val collect_common_twiddle : bool ref
val collect_common_inputs : bool ref
val verbose : bool ref
val name : string ref

val imul_to_lea_limit : int ref
val do_debug_output : bool ref
val vectsteps_limit : int ref

type amd_processor = 
  | AMD_K6
  | AMD_K7

val target_processor : amd_processor ref
