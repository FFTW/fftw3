(*
 * Copyright (c) 1997-1999 Massachusetts Institute of Technology
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

(* Data types and functions for dealing with variables in symbolic
 * expressions and the abstract syntax tree. *)

(* Variables fall into one of four categories: temporary variables
 * (which the generator can add or delete at will), named (fixed)
 * variables, and the real/imaginary parts of complex arrays.  Arrays
 * can be either input arrays, output arrays, or arrays of precomputed
 * twiddle factors (roots of unity). *)

type array = Input | Output | Twiddle

let arrayToString = function 
  | Input   -> "in" 
  | Output  -> "out" 
  | Twiddle -> "tw"

type variable =
  | Temporary of int
  | Named of string
  | RealArrayElem of (array * int)
  | ImagArrayElem of (array * int)

let make_temporary =
  let tmp_count = ref 0
  in fun () -> begin
    tmp_count := !tmp_count + 1;
    Temporary !tmp_count
  end

let is_temporary = function
  | Temporary _ -> true
  | _ -> false

let is_real = function
  | RealArrayElem _ -> true
  | _ -> false

let is_imag = function
  | ImagArrayElem _ -> true
  | _ -> false

let is_output = function
  | RealArrayElem (Output, _) -> true
  | ImagArrayElem (Output, _) -> true
  | _ -> false

let is_input = function
  | RealArrayElem (Input, _) -> true
  | ImagArrayElem (Input, _) -> true
  | _ -> false

let is_twiddle = function
  | RealArrayElem (Twiddle, _) -> true
  | ImagArrayElem (Twiddle, _) -> true
  | _ -> false

let is_locative x = (is_input x) || (is_output x)
let is_constant = is_twiddle

let same = (=)

let hash = function
  | RealArrayElem(Input,i)   ->  i * 8
  | ImagArrayElem(Input,i)   -> -i * 8 + 1
  | RealArrayElem(Output,i)  ->  i * 8 + 2
  | ImagArrayElem(Output,i)  -> -i * 8 + 3
  | RealArrayElem(Twiddle,i) ->  i * 8 + 4
  | ImagArrayElem(Twiddle,i) -> -i * 8 + 5
  | _ -> 0
  
let similar a b = 
  same a b ||
  (match (a, b) with
     | (RealArrayElem (a1, k1), ImagArrayElem (a2, k2)) -> a1 = a2 && k1 = k2
     | (ImagArrayElem (a1, k1), RealArrayElem (a2, k2)) -> a1 = a2 && k1 = k2
     | _ -> false)
    
(* true if assignment of a clobbers variable b *)
let clobbers a b = match (a, b) with
  | (RealArrayElem (Output, k1), RealArrayElem (Input, k2)) -> k1 = k2
  | (ImagArrayElem (Output, k1), ImagArrayElem (Input, k2)) -> k1 = k2
  | _ -> false

(* true if a is the real part and b the imaginary of the same array *)
let real_imag a b = match (a, b) with
  | (RealArrayElem (a1, k1), ImagArrayElem (a2, k2)) -> a1 = a2 && k1 = k2 
  | _ -> false

(* true if a and b are elements of the same array, and a has smaller index *)
let increasing_indices a b = match (a, b) with
  | (RealArrayElem (a1, k1), RealArrayElem (a2, k2)) -> a1 = a2 && k1 < k2 
  | (RealArrayElem (a1, k1), ImagArrayElem (a2, k2)) -> a1 = a2 && k1 < k2 
  | (ImagArrayElem (a1, k1), RealArrayElem (a2, k2)) -> a1 = a2 && k1 < k2 
  | (ImagArrayElem (a1, k1), ImagArrayElem (a2, k2)) -> a1 = a2 && k1 < k2 
  | _ -> false

let access array k = (RealArrayElem (array, k),  ImagArrayElem (array, k))

let access_input = access Input
let access_output = access Output
let access_twiddle = access Twiddle

let make_named name = Named name

let unparse = function
  | Temporary x -> "T" ^ (string_of_int x)
  | Named s -> s
  | RealArrayElem (a, x) -> "Re(" ^ (arrayToString a) ^ "[" ^
      (string_of_int x) ^ "])"
  | ImagArrayElem (a, x) -> "Im(" ^ (arrayToString a) ^ "[" ^
      (string_of_int x) ^ "])"
							  
