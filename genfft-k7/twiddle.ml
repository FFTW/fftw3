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
(* $Id: twiddle.ml,v 1.8 2006-01-05 03:04:27 stevenj Exp $ *)

(* policies for loading/computing twiddle factors *)
open Complex
open Util

type twop = TW_FULL | TW_COS | TW_SIN | TW_TAN | TW_NEXT

let optoint = function
  | TW_COS -> 0
  | TW_SIN -> 1
  | TW_TAN -> 2
  | TW_NEXT -> 3
  | TW_FULL -> 4

let optostring = function
  | TW_COS -> "TW_COS"
  | TW_SIN -> "TW_SIN"
  | TW_TAN -> "TW_TAN"
  | TW_NEXT -> "TW_NEXT"
  | TW_FULL -> "TW_FULL"

type twinstr = (twop * int * int)

let twinstr_to_c_string l =
  let one (op, a, b) = Printf.sprintf "{ %s, %d, %d }" (optostring op) a b
  in let rec loop first = function
    | [] -> ""
    | a :: b ->  (if first then "\n" else ",\n") ^ (one a) ^ (loop false b)
  in "{" ^ (loop true l) ^ "}"

let twinstr_to_asm_string l =
  let one (op, a, b) = 
    Printf.sprintf "\t.byte %d\n\t.byte %d\n\t.value %d\n" (optoint op) a b
  in List.fold_left (^) "" (List.map one l)
  
let square x = 
  if (!Magic.wsquare) then
    wsquare x
  else
    times x x

let rec is_pow_2 n = 
  n = 1 || ((n mod 2) = 0 && is_pow_2 (n / 2))

let rec log2 n = if n = 1 then 0 else 1 + log2 (n / 2)

let rec largest_power_of_2_smaller_than i =
  if (is_pow_2 i) then i
  else largest_power_of_2_smaller_than (i - 1)

let rec_array n f =
  let g = ref (fun i -> Complex.zero) in
  let a = Array.init n (fun i -> lazy (!g i)) in
  let h i = f (fun i -> Lazy.force a.(i)) i in
  begin
    g := h;
    h
  end

let load_reim sign w i = 
  if sign = 1 then w i else Complex.conj (w i)

(* various policies for computing/loading twiddle factors *)

(* load all twiddle factors *)
let twiddle_policy_load_all =
  let bytwiddle n sign w f i =
    if i = 0 then 
      f i
    else
      Complex.times (load_reim sign w (i - 1)) (f i)
  and twidlen n = 2 * (n - 1)
  and twdesc r = [(TW_FULL, 0, r);(TW_NEXT, 1, 0)]
  in bytwiddle, twidlen, twdesc

(* shorthand for policies that only load W[0] *)
let policy_one mktw =
  let bytwiddle n sign w f = 
    let g = (mktw n (load_reim sign w)) in
    fun i -> Complex.times (g i) (f i)
  and twidlen n = 2
  and twdesc n = [(TW_COS, 0, 1); (TW_SIN, 0, 1); (TW_NEXT, 1, 0)]
  in bytwiddle, twidlen, twdesc
    
(* compute w^n = w w^{n-1} *)
let twiddle_policy_iter =
  policy_one (fun n ltw ->
    rec_array n (fun self i ->
      if i = 0 then Complex.one
      else if i = 1 then ltw (i - 1)
      else times (self (i - 1)) (self 1)))

(*
 * if n is even, compute w^n = (w^{n/2})^2, else
 *  w^n = w w^{n-1}
 *)
let twiddle_policy_square1 =
  policy_one (fun n ltw ->
    rec_array n (fun self i ->
      if i = 0 then Complex.one
      else if i = 1 then ltw (i - 1)
      else if ((i mod 2) == 0) then
	square (self (i / 2))
      else times (self (i - 1)) (self 1)))

(*
 * if n is even, compute w^n = (w^{n/2})^2, else
 * compute  w^n from w^{n-1}, w^{n-2}, and w
 *)
let twiddle_policy_square2 =
  policy_one (fun n ltw ->
    rec_array n (fun self i ->
      if i = 0 then Complex.one
      else if i = 1 then ltw (i - 1)
      else if ((i mod 2) == 0) then
	square (self (i / 2))
      else 
	wthree (self (i - 1)) (self (i - 2)) (self 1)))

(*
 * if n is even, compute w^n = (w^{n/2})^2, else
 *  w^n = w^{floor(n/2)} w^{ceil(n/2)}
 *)
let twiddle_policy_square3 =
  policy_one (fun n ltw ->
    rec_array n (fun self i ->
      if i = 0 then Complex.one
      else if i = 1 then ltw (i - 1)
      else if ((i mod 2) == 0) then
	square (self (i / 2))
      else times (self (i / 2)) (self (i - i / 2))))

(*
 * twiddle policy used by Takuya Ooura in his code.
 * if i is a power of two, then w^i = (w^(i/2)) ^2
 * else let x = largest power of 2 less than i in
 *      let y = i - x in
 *      compute w^{x+y} from w^{x-y}, w^{x}, and w^{y} using
 *      reflection formula
 *)
let twiddle_policy_ooura =
  policy_one (fun n ltw ->
    rec_array n (fun self i ->
      if i = 0 then Complex.one
      else if (i == 1) then ltw (i - 1)
      else if (is_pow_2 i) then
	square (self (i / 2))
      else
	let x = largest_power_of_2_smaller_than i in
	let y = i - x in
	wreflect (self (x - y)) (self x) (self y)))

let current_twiddle_policy = ref twiddle_policy_load_all

let twiddle_policy () = !current_twiddle_policy

let set_policy x = Arg.Unit (fun () -> current_twiddle_policy := x)

let undocumented = " Undocumented twiddle policy"

let speclist = [
  "-twiddle-load-all", set_policy twiddle_policy_load_all, undocumented;
  "-twiddle-iter", set_policy twiddle_policy_iter, undocumented;
  "-twiddle-square1", set_policy twiddle_policy_square1, undocumented;
  "-twiddle-square2", set_policy twiddle_policy_square2, undocumented;
  "-twiddle-square3", set_policy twiddle_policy_square3, undocumented;
  "-twiddle-ooura", set_policy twiddle_policy_ooura, undocumented;
] 
