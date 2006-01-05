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
(* $Id: fft.ml,v 1.4 2006-01-05 03:04:27 stevenj Exp $ *)

let cvsid = "$Id: fft.ml,v 1.4 2006-01-05 03:04:27 stevenj Exp $"

(* This is the part of the generator that actually computes the FFT
   in symbolic form *)

open Complex
open Util

(* choose a suitable factor of n *)
let choose_factor n =
  (* first choice: i such that gcd(i, n / i) = 1, i as big as possible *)
  let choose1 n =
    let rec loop i f =
      if (i * i > n) then f
      else if ((n mod i) == 0 && gcd i (n / i) == 1) then loop (i + 1) i
      else loop (i + 1) f
    in loop 1 1

  (* second choice: the biggest factor i of n, where i < sqrt(n), if any *)
  and choose2 n =
    let rec loop i f =
      if (i * i > n) then f
      else if ((n mod i) == 0) then loop (i + 1) i
      else loop (i + 1) f
    in loop 1 1

  in let i = choose1 n in
  if (i > 1) then i
  else choose2 n

  
let rec dft_prime sign n input = 
  let sum filter i =
    sigma 0 n (fun j ->
      let coeff = filter (exp n (sign * i * j))
      in coeff @* (input j)) in
  let computation_even = array n (sum identity)
  and computation_odd =
    let sumr = array n (sum real)
    and sumi = array n (sum ((times Complex.i) @@ imag)) in
    array n (fun i ->
      if (i = 0) then
	(* expose some common subexpressions *)
	input 0 @+ 
	sigma 1 ((n + 1) / 2) (fun j -> input j @+ input (n - j))
      else
	let i' = min i (n - i) in
	if (i < n - i) then 
	  sumr i' @+ sumi i'
	else
	  sumr i' @- sumi i') in
  if (n >= !Magic.rader_min) then
    dft_rader sign n input
  else if (n == 2) then
    computation_even
  else
    computation_odd 


and dft_rader sign p input =
  let half = 
    let one_half = inverse_int 2 in
    times one_half

  and make_product n a b =
    let scale_factor = inverse_int n in
    array n (fun i -> a i @* (scale_factor @* b i)) in

  (* generates a convolution using ffts.  (all arguments are the
     same as to gen_convolution, below) *)
  let gen_convolution_by_fft n a b addtoall =
    let fft_a = dft 1 n a
    and fft_b = dft 1 n b in 

    let fft_ab = make_product n fft_a fft_b
    and dc_term i = if (i == 0) then addtoall else zero in

    let fft_ab1 = array n (fun i -> fft_ab i @+ dc_term i)
    and sum = fft_a 0 in
    let conv = dft (-1) n fft_ab1 in
    (sum, conv)

  (* alternate routine for convolution.  Seems to work better for
     small sizes.  I have no idea why. *)
  and gen_convolution_by_fft_alt n a b addtoall =
    let ap = array n (fun i -> half (a i @+ a ((n - i) mod n)))
    and am = array n (fun i -> half (a i @- a ((n - i) mod n)))
    and bp = array n (fun i -> half (b i @+ b ((n - i) mod n)))
    and bm = array n (fun i -> half (b i @- b ((n - i) mod n)))
    in

    let fft_ap = dft 1 n ap
    and fft_am = dft 1 n am
    and fft_bp = dft 1 n bp
    and fft_bm = dft 1 n bm in

    let fft_abpp = make_product n fft_ap fft_bp
    and fft_abpm = make_product n fft_ap fft_bm
    and fft_abmp = make_product n fft_am fft_bp
    and fft_abmm = make_product n fft_am fft_bm 
    and sum = fft_ap 0 @+ fft_am 0
    and dc_term i = if (i == 0) then addtoall else zero in

    let fft_ab1 = array n (fun i -> (fft_abpp i @+ fft_abmm i) @+ dc_term i)
    and fft_ab2 = array n (fun i -> fft_abpm i @+ fft_abmp i) in
    let conv1 = dft (-1) n fft_ab1 
    and conv2 = dft (-1) n fft_ab2 in
    let conv = array n (fun i ->
      conv1 i @+ conv2 i) in
    (sum, conv) 

    (* generator of assignment list assigning conv to the convolution of
       a and b, all of which are of length n.  addtoall is added to
       all of the elements of the result.  Returns (sum, convolution) pair
       where sum is the sum of the elements of a. *)

  in let gen_convolution = 
    if (p <= !Magic.alternate_convolution) then 
      gen_convolution_by_fft_alt
    else
      gen_convolution_by_fft

  (* fft generator for prime n = p using Rader's algorithm for
     turning the fft into a convolution, which then can be
     performed in a variety of ways *)
  in  
    let g = find_generator p in
    let ginv = pow_mod g (p - 2) p in
    let input_perm = array p (fun i -> input (pow_mod g i p))
    and omega_perm = array p (fun i -> exp p (sign * (pow_mod ginv i p)))
    and output_perm = array p (fun i -> pow_mod ginv i p)
    in let (sum, conv) = 
      (gen_convolution (p - 1)  input_perm omega_perm (input 0))
    in array p (fun i ->
      if (i = 0) then
	input 0 @+ sum
      else
	let i' = suchthat 0 (fun i' -> i = output_perm i')
	in conv i')

and dft sign n input =
  let rec cooley_tukey sign n1 n2 input =
    let tmp1 = 
      array n2 (fun i2 -> 
	dft sign n1 (fun i1 -> input (i1 * n2 + i2))) in
    let tmp2 =  
      array n1 (fun i1 ->
	array n2 (fun i2 ->
	  exp n (sign * i1 * i2) @* tmp1 i2 i1)) in
    let tmp3 = array n1 (fun i1 -> dft sign n2 (tmp2 i1)) in
    (fun i -> tmp3 (i mod n1) (i / n1))

  (*
   * This is "exponent -1" split-radix by Dan Bernstein.
   *)
  and split_radix_dit sign n input =
    let f0 = dft sign (n / 2) (fun i -> input (i * 2))
    and f10 = dft sign (n / 4) (fun i -> input (i * 4 + 1))
    and f11 = dft sign (n / 4) (fun i -> input ((n + i * 4 - 1) mod n)) in
    let g10 = array n (fun k ->
      exp n (sign * k) @* f10 (k mod (n / 4)))
    and g11 = array n (fun k ->
      exp n (- sign * k) @* f11 (k mod (n / 4))) in
    let g1 = array n (fun k -> g10 k @+ g11 k) in
    array n (fun k -> f0 (k mod (n / 2)) @+ g1 k)

  and split_radix_dif sign n input =
    let n2 = n / 2 and n4 = n / 4 in
    let x0 = array n2 (fun i -> input i @+ input (i + n2))
    and x10 = array n4 (fun i -> input i @- input (i + n2))
    and x11 = array n4 (fun i ->
	input (i + n4) @- input (i + n2 + n4)) in
    let x1 k i = 
      exp n (k * i * sign) @* (x10 i @+ exp 4 (k * sign) @* x11 i) in
    let f0 = dft sign n2 x0 
    and f1 = array 4 (fun k -> dft sign n4 (x1 k)) in
    array n (fun k ->
      if k mod 2 = 0 then f0 (k / 2)
      else let k' = k mod 4 in f1 k' ((k - k') / 4))

  and prime_factor sign n1 n2 input =
    let tmp1 = array n2 (fun i2 ->
      dft sign n1 (fun i1 -> input ((i1 * n2 + i2 * n1) mod n)))
    in let tmp2 = array n1 (fun i1 ->
      dft sign n2 (fun k2 -> tmp1 k2 i1))
    in fun i -> tmp2 (i mod n1) (i mod n2)

  in let algorithm sign n =
    let r = choose_factor n in
    if List.mem n !Magic.rader_list then
      (* special cases *)
      dft_rader sign n
    else if (r == 1) then  (* n is prime *)
      dft_prime sign n
    else if (gcd r (n / r)) == 1 then
      prime_factor sign r (n / r)
    else if (n mod 4 = 0 && n > 4) then
      (if !Magic.dif_split_radix then
	split_radix_dif sign n
      else
	split_radix_dit sign n)
    else 
      cooley_tukey sign r (n / r)
  in
  array n (algorithm sign n input)
