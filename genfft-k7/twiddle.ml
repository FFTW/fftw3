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

(* $Id: twiddle.ml,v 1.1 2002-06-14 10:56:15 athena Exp $ *)

(* This file implements various policies for either loading the twiddle
 * factors according to various algorithms. *)

open Complex
open Util

let square x = 
  if !Magic.use_wsquare then
    wsquare x
  else
    times x x

(* various policies for computing/loading twiddle factors *)
(* load all twiddle factors *)
let twiddle_policy_load_all =
  let twiddle_expression n i _ =
    load_var (access_twiddle (i - 1))
  and num_twiddle n = (n - 1)
  and twiddle_order n = forall_flat 1 n (fun i -> [i])
  in twiddle_expression, num_twiddle, twiddle_order

(*
 * if n is even, compute w^n = (w^{n/2})^2, else
 * load it
 *)
let twiddle_policy_load_odd =
  let twiddle_expression n i twiddles =
    if ((i mod 2) == 0) then
      square (twiddles (i / 2))
    else load_var (access_twiddle ((i - 1) / 2))
  and num_twiddle n = (n / 2)
  and twiddle_order n = forall_flat 1 n (fun i -> 
    if ((i mod 2) == 1) then [i] else [])
  in twiddle_expression, num_twiddle, twiddle_order

(* compute w^n = w w^{n-1} *)
let twiddle_policy_iter =
  let twiddle_expression n i twiddles =
    if (i == 1) then load_var (access_twiddle (i - 1))
    else times (twiddles (i - 1)) (twiddles 1)
  and num_twiddle n = 1
  and twiddle_order n = [1]
  in twiddle_expression, num_twiddle, twiddle_order

(*
 * if n is even, compute w^n = (w^{n/2})^2, else
 *  w^n = w w^{n-1}
 *)
let twiddle_policy_square1 =
  let twiddle_expression n i twiddles =
    if (i == 1) then load_var (access_twiddle (i - 1))
    else if ((i mod 2) == 0) then
      square (twiddles (i / 2))
    else times (twiddles (i - 1)) (twiddles 1)
  and num_twiddle n = 1
  and twiddle_order n = [1]
  in twiddle_expression, num_twiddle, twiddle_order

(*
 * if n is even, compute w^n = (w^{n/2})^2, else
 * compute  w^n from w^{n-1}, w^{n-2}, and w
 *)
let twiddle_policy_square2 =
  let twiddle_expression n i twiddles =
    if (i == 1) then load_var (access_twiddle (i - 1))
    else if ((i mod 2) == 0) then
      square (twiddles (i / 2))
    else 
      wthree (twiddles (i - 1)) (twiddles (i - 2)) (twiddles 1)
  and num_twiddle n = 1
  and twiddle_order n = [1]
  in twiddle_expression, num_twiddle, twiddle_order

(*
 * if n is even, compute w^n = (w^{n/2})^2, else
 *  w^n = w^{floor(n/2)} w^{ceil(n/2)}
 *)
let twiddle_policy_square3 =
  let twiddle_expression n i twiddles =
    if (i == 1) then load_var (access_twiddle (i - 1))
    else if ((i mod 2) == 0) then
      square (twiddles (i / 2))
    else times (twiddles (i / 2)) (twiddles (i - i / 2))
  and num_twiddle n = 1
  and twiddle_order n = [1]
  in twiddle_expression, num_twiddle, twiddle_order

let twiddle_policy () = match !Magic.twiddle_policy with
  | Magic.TWIDDLE_LOAD_ALL -> twiddle_policy_load_all
  | Magic.TWIDDLE_ITER     -> twiddle_policy_iter
  | Magic.TWIDDLE_LOAD_ODD -> twiddle_policy_load_odd
  | Magic.TWIDDLE_SQUARE1  -> twiddle_policy_square1
  | Magic.TWIDDLE_SQUARE2  -> twiddle_policy_square2
  | Magic.TWIDDLE_SQUARE3  -> twiddle_policy_square3

