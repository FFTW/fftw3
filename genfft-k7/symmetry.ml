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

(* $Id: symmetry.ml,v 1.1 2002-06-14 10:56:15 athena Exp $ *)

(* various kinds of symmetries *)

open Complex 
open Util

(*
 * symmetries are encoded as symmetries of the *input*.  A symmetry
 * determines 
 * 1) the symmetry of the output (osym)
 * 2) symmetries at intermediate stages of divide and conquer or Rader
 *    (isym1 and isym2)
 *)
type symmetry = {
  apply: int -> (int -> Complex.expr) -> int -> Complex.expr;
  store: int -> (int -> Complex.expr) -> int -> Exprdag.node list;
  osym: symmetry;
  isym1: symmetry;
  isym2: symmetry}

(* no symmetry *)
let rec no_sym = {
  isym1 = no_sym; 
  isym2 = no_sym;
  osym = no_sym;
  store = (fun _ f i -> store_var (access_output i) (f i));
  apply = fun _ f -> f
}

(* the crazy symmetry of the intermediate elements of 
   the hc2hc_forward transform. *)
and middle_hc2hc_forward_sym = {
  osym = middle_hc2hc_forward_sym;
  isym1 = no_sym;
  isym2 = no_sym;
  store = (fun n f i -> 
    if (i < n - i) then
      store_var (access_output i) (f i)
    else
      store_var (access_output i) (swap_re_im (conj (f i))));
  apply = fun _ f -> f
}

(* the crazy symmetry of the intermediate elements of 
   the hc2hc_backward transform. *)
and middle_hc2hc_backward_sym = {
  osym = no_sym;
  isym1 = no_sym;
  isym2 = no_sym;
  store = (fun _ -> failwith "middle_hc2hc_backward_sym");
  apply = fun n f i ->
    if (i < n - i) then
      (f i)
    else
      conj (swap_re_im (f i))
}

(* the crazy symmetry of the n/2-th element of 
   the hc2hc_forward transform. *)
and final_hc2hc_forward_sym = {
  osym = final_hc2hc_forward_output_sym;
  isym1 = real_sym;
  isym2 = no_sym;
  store = (fun n f i ->
    if (2 * i < n) then store_real (access_output i) (f i)
    else []);
  apply = fun n f i ->
    if (2 * i < n) then real (f i)
    else uminus (real (f (i - n/2)))
} 

and final_hc2hc_backward_sym = {
  osym = final_hc2hc_forward_sym;
  isym1 = no_sym;
  isym2 = no_sym;
  store = (fun _ -> failwith "final_hc2hc_backward_sym");
  apply = (fun n f i -> 
    if (i mod 2 == 0) then zero
    else (
      let i' = (i - 1) / 2
      and n' = n / 2 
      in
      if (2 * i' < n' - 1) then (f i')
      else if (2 * i' == n' - 1) then 
	real (f i')
      else conj (f (n' - 1 - i'))
    ))
}

and final_hc2hc_forward_output_sym = {
  osym = final_hc2hc_forward_sym;
  isym1 = no_sym;
  isym2 = no_sym;
  store = (fun n f i -> 
    if (i mod 2 == 0) then []
    else (
      let i' = (i - 1) / 2
      and n' = n / 2 
      in
      if (2 * i' < n' - 1) then 
	store_var (access_output i') (times (inverse_int 2) (f i))
      else if (2 * i' == n' - 1) then 
	store_real (access_output i') (times (inverse_int 2) (f i))
      else []
    ));
  apply = fun _ f -> f
}

(* real input data *)
and real_sym = {
  osym = hermitian_sym;
  isym1 = real_sym;
  isym2 = no_sym;
  store = (fun _ f i -> store_real (access_output i) (f i));
  apply = fun _ f -> real @@ f
}

(* imaginary input data *)
and imag_sym = {
  osym = antihermitian_sym;
  isym1 = imag_sym;
  isym2 = no_sym;
  store = (fun _ f i -> store_imag (access_output i) (f i));
  apply = fun _ f -> imag @@ f
}

(* real, even input data *)
and realeven_sym = {
  osym = realeven_sym;
  isym1 = real_sym;
  isym2 = hermitian_sym;
  store = (fun n f i -> 
    if (i <= n - i) then store_real (access_output i) (f i)
    else []);
  apply = fun n f i -> 
    if (i <= n - i) then real (f i)
    else real (f (n - i))
}

(* imaginary, even input data *)
and imageven_sym = {
  osym = imageven_sym;
  isym1 = imag_sym;
  isym2 = antihermitian_sym;
  store = (fun n f i -> 
    if (i <= n - i) then store_imag (access_output i) (f i)
    else []);
  apply = fun n f i -> 
    if (i <= n - i) then imag (f i)
    else imag (f (n - i))
}

(* real, odd input data *)
and realodd_sym = {
  osym = imagodd_sym;
  isym1 = real_sym;
  isym2 = antihermitian_sym;
  store = (fun n f i -> 
    if ((i > 0) && (i < n - i)) then store_real (access_output i) (f i)
    else []);
  apply = fun n f i ->
    if (i == 0) then zero
    else if (i < n - i)  then real (f i)
    else if (i > n - i) then real (uminus (f (n - i)))
    else zero
}

(* imaginary, odd input data *)
and imagodd_sym = {
  osym = realodd_sym;
  isym1 = imag_sym;
  isym2 = hermitian_sym;
  store = (fun n f i -> 
    if ((i > 0) && (i < n - i)) then store_imag (access_output i) (f i)
    else []);
  apply = fun n f i ->
    if (i == 0) then zero
    else if (i < n - i)  then imag (f i)
    else if (i > n - i) then imag (uminus (f (n - i)))
    else zero
}


(* halfcomplex/anti-hermitian input data *)
and antihermitian_sym = {
  osym = imag_sym;
  isym1 = no_sym;
  isym2 = antihermitian_sym;
  apply = (fun n f i ->
    if (i = 0) then imag (f 0)
    else if (i < n - i)  then (f i)
    else if (i > n - i)  then uminus (conj (f (n - i)))
    else imag (f i));
  store = fun n f i ->
    if (i = 0) then store_imag (access_output i) (f i)
    else if (i < n - i) then store_var (access_output i) (f i)
    else if (i == n - i) then store_imag (access_output i) (f i)
    else []
} 

(* halfcomplex/hermitian input data *)
and hermitian_sym = {
  osym = real_sym;
  isym1 = no_sym;
  isym2 = hermitian_sym;
  apply = (fun n f i ->
    if (i = 0) then real (f 0)
    else if (i < n - i)  then (f i)
    else if (i > n - i)  then conj (f (n - i))
    else real (f i));
  store = fun n f i ->
    if (i = 0) then store_real (access_output i) (f i)
    else if (i < n - i) then store_var (access_output i) (f i)
    else if (i == n - i) then store_real (access_output i) (f i)
    else []
} 


(* symmetric input data, used by rader *)
and symmetric_sym = {
  osym = symmetric_sym;
  isym1 = no_sym;
  isym2 = no_sym;
  apply = (fun n f i ->
    if (i < n - i) then (f i)
    else (f (n - i)));
  store = (fun _ -> failwith "symmetric_sym")
} 

(* anti-symmetric input data, used by rader *)
and anti_symmetric_sym = {
  osym = anti_symmetric_sym;
  isym1 = no_sym;
  isym2 = no_sym;
  apply = (fun n f i ->
    if (i == 0) then zero
    else if (i < n - i)  then (f i)
    else if (i > n - i) then uminus (f (n - i))
    else zero);
  store = (fun _ -> failwith "anti_symmetric_sym")
} 

(* real, even-2 input data (even about n=-1/2, not n=0). *)
and realeven2_input_sym = {
  osym = realeven2_output_sym;
  isym1 = real_sym;
  isym2 = hermitian_sym;
  store = (fun _ -> failwith "realeven2_input_sym");
  apply = fun n f i -> 
    if ((i mod 2) == 0) then zero
    else if (i <= n - i) then
      real (f ((i - 1) / 2))
    else
      real (f (n/2 - 1 - (i - 1)/2))
}

(* real, even-2 output data (even about n=-1/2, not n=0).  We have multiplied
   output[k] by omega^(k/2); the result is real, odd, and anti-periodic. *)
and realeven2_output_sym = {
  osym = no_sym;
  isym1 = no_sym;
  isym2 = no_sym;
  store = (fun n f i -> 
    if (4 * i < n) then store_real (access_output i) (f i)
    else []);
  apply = (fun n f i -> f i)
}

(* real, odd-2 input data (odd about n=-1/2, not n=0). *)
and realodd2_input_sym = {
  osym = realodd2_output_sym;
  isym1 = real_sym;
  isym2 = antihermitian_sym;
  store = (fun _ -> failwith "realodd2_input_sym");
  apply = fun n f i -> 
    if ((i mod 2) == 0) then zero
    else if (i < n - i) then
      real (f ((i - 1) / 2))
    else if (i == n - i) then zero
    else uminus (real (f (n/2 - 1 - (i - 1)/2)))
}

(* real, odd-2 output data (odd about n=-1/2, not n=0).  We have multiplied
   output[k] by omega^(k/2); the result is imaginary, even,
   and anti-periodic. *)
and realodd2_output_sym = {
  osym = no_sym;
  isym1 = no_sym;
  isym2 = no_sym;
  store = (fun n f i -> 
    if (i > 0 && 4 * i <= n) then store_imag (access_output i) (f i)
    else []);
  apply = (fun n f i -> f i)
}
