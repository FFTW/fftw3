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

(* $Id: fft.ml,v 1.1 2002-06-14 10:56:15 athena Exp $ *)
let cvsid = "$Id: fft.ml,v 1.1 2002-06-14 10:56:15 athena Exp $"

(* This is the part of the generator that actually computes the FFT,
   in symbolic form, and outputs the assignment lists (which assign
   the DFT of the input array to the output array). *)

open Complex
open Util
open Lazy
open Twiddle
open Symmetry

let (@*) = times
let (@+) a b = plus [a; b]
let (@-) a b = plus [a; uminus b]

(* choose a suitable factor of n *)
let choose_factor n =
  (* first choice: i such that gcd(i, n / i) = 1, i as big as possible *)
  let choose1 n =
    let rec loop i f =
      if (i * i > n) then f
      else if ((n mod i) == 0 && gcd i (n / i) == 1) then loop (i + 1) i
      else loop (i + 1) f
    in loop 1 1

  (* second choice: the biggest factor i of n, where i < sqrt(n) *)
  and choose2 n =
    let rec loop i f =
      if (i * i > n) then f
      else if ((n mod i) == 0) then loop (i + 1) i
      else loop (i + 1) f
    in loop 1 1

  in let i = choose1 n in
  if (i > 1) then i
  else choose2 n

let freeze n f =
  let a = Array.init n (fun i -> lazy (f i))
  in fun i -> force a.(i)
  
let rec fftgen_prime n input sign = 
  let sum filter i =
    sigma 0 n (fun j ->
      let coeff = filter (exp n (sign * i * j))
      in coeff @* (input j)) in
  let computation_even = freeze n (sum identity)
  and computation_odd =
    let sumr = freeze n (sum real)
    and sumi = freeze n (sum imag) in
    freeze n (fun i ->
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
    fftgen_rader n input sign
  else if (n == 2) then
    computation_even
  else
    computation_odd 


and fftgen_rader p input sign =
  let half = 
    let one_half = inverse_int 2 in
    times one_half

  and make_product n a b =
    let scale_factor = inverse_int n in
    freeze n (fun i ->
      a i @* (scale_factor @* b i)) in

  (* generates a convolution using ffts.  (all arguments are the
     same as to gen_convolution, below) *)
  let gen_convolution_by_fft n a b addtoall =
    let fft_a = fftgen n no_sym a 1
    and fft_b = fftgen n no_sym b 1 in 

    let fft_ab = make_product n fft_a fft_b
    and dc_term i = if (i == 0) then addtoall else zero in

    let fft_ab1 = freeze n (fun i -> fft_ab i @+ dc_term i)
    and sum = (fft_a 0) in
    let conv = fftgen n no_sym fft_ab1 (-1) in
    (sum, conv)

  (* alternate routine for convolution.  Seems to work better for
     small sizes.  I have no idea why. *)
  and gen_convolution_by_fft_alt n a b addtoall =
    let ap = freeze n (fun i -> half (a i @+ a ((n - i) mod n)))
    and am = freeze n (fun i -> half (a i @- a ((n - i) mod n)))
    and bp = freeze n (fun i -> half (b i @+ b ((n - i) mod n)))
    and bm = freeze n (fun i -> half (b i @- b ((n - i) mod n)))
    in

    let fft_ap = fftgen n symmetric_sym ap 1
    and fft_am = fftgen n anti_symmetric_sym am 1
    and fft_bp = fftgen n symmetric_sym bp 1
    and fft_bm = fftgen n anti_symmetric_sym bm 1 in

    let fft_abpp = make_product n fft_ap fft_bp
    and fft_abpm = make_product n fft_ap fft_bm
    and fft_abmp = make_product n fft_am fft_bp
    and fft_abmm = make_product n fft_am fft_bm 
    and sum = fft_ap 0 @+ fft_am 0
    and dc_term i = if (i == 0) then addtoall else zero in

    let fft_ab1 = freeze n (fun i ->
      (fft_abpp i @+ fft_abmm i) @+ dc_term i)
    and fft_ab2 = freeze n (fun i ->
      fft_abpm i @+ fft_abmp i) in
    let conv1 = fftgen n symmetric_sym fft_ab1 (-1) 
    and conv2 = fftgen n anti_symmetric_sym fft_ab2 (-1)  in
    let conv = freeze n (fun i ->
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
    let input_perm = freeze p (fun i -> input (pow_mod g i p))
    and omega_perm = freeze p (fun i -> exp p (sign * (pow_mod ginv i p)))
    and output_perm = freeze p (fun i -> pow_mod ginv i p)
    in let (sum, conv) = 
      (gen_convolution (p - 1)  input_perm omega_perm (input 0))
    in freeze p (fun i ->
      if (i = 0) then
	input 0 @+ sum
      else
	let i' = suchthat 0 (fun i' -> i = output_perm i')
	in conv i')

and fftgen n sym =
  let rec cooley_tukey n1 n2 input sign =
    let tmp1 = freeze n2 (fun i2 ->
      fftgen n1 sym.isym1 (fun i1 -> input (i1 * n2 + i2)) sign) in
    let tmp2 =  freeze n1 (fun i1 ->
      freeze n2 (fun i2 ->
	exp n (sign * i1 * i2) @* tmp1 i2 i1)) in
    let tmp3 = freeze n1 (fun i1 ->
      fftgen n2 sym.isym2 (tmp2 i1) sign) in
    (fun i -> tmp3 (i mod n1) (i / n1))

  (* This is "exponent -1" split-radix by Dan Bernstein *)
  and split_radix_dit n input sign =
    let f0 = fftgen (n / 2) sym.isym1 (fun i -> input (i * 2)) sign
    and f10 = fftgen (n / 4) sym.isym1 (fun i -> input (i * 4 + 1)) sign
    and f11 = fftgen (n / 4) sym.isym1 (fun i -> input 
	((n + i * 4 - 1) mod n)) sign in
    let g10 = freeze n (fun k ->
      exp n (sign * k) @* f10 (k mod (n / 4)))
    and g11 = freeze n (fun k ->
      exp n (- sign * k) @* f11 (k mod (n / 4))) in
    let g1 = freeze n (fun k -> g10 k @+ g11 k) in
    freeze n (fun k -> f0 (k mod (n / 2)) @+ g1 k)

  and split_radix_dif n input sign =
    let n2 = n / 2 and n4 = n / 4 in
    let x0 = freeze n2 (fun i -> input i @+ input (i + n2))
    and x10 = freeze n4 (fun i -> input i @- input (i + n2))
    and x11 = freeze n4 (fun i ->
	input (i + n4) @- input (i + n2 + n4)) in
    let x1 k i = 
      exp n (k * i * sign) @* 
	(x10 i @+ exp 4 (k * sign) @* x11 i) in
    let f0 = fftgen n2 sym.isym2 x0 sign 
    and f1 = freeze 4 (fun k -> fftgen n4 sym.isym2 (x1 k) sign) in
    freeze n (fun k ->
      if k mod 2 = 0 then f0 (k / 2)
      else let k' = k mod 4 in f1 k' ((k - k') / 4))

  and prime_factor n1 n2 input sign =
    let tmp1 = freeze n2 (fun i2 ->
      fftgen n1 sym.isym1 (fun i1 -> input ((i1 * n2 + i2 * n1) mod n)) sign)
    in let tmp2 = freeze n1 (fun i1 ->
      fftgen n2 sym.isym2 (fun k2 -> tmp1 k2 i1) sign)
    in fun i -> tmp2 (i mod n1) (i mod n2)

  in let r = choose_factor n 
  in let fft =
    if List.mem n !Magic.rader_list then
      (* special cases *)
      fftgen_rader n
    else if (r == 1) then  (* n is prime *)
      fftgen_prime n
    else if (gcd r (n / r)) == 1 then
      prime_factor r (n / r)
    else if (n mod 4 = 0 && n > 4) then
      (if sym == hermitian_sym then
	split_radix_dif n
      else
	split_radix_dit n)
    else
      cooley_tukey r (n / r)
  in fun input sign ->
    freeze n (sym.osym.apply n (fft (sym.apply n input) sign))

type direction = FORWARD | BACKWARD

let sign_of_dir = function
    FORWARD -> (-1)
  | BACKWARD -> 1

let conj_of_dir = function
    FORWARD -> identity
  | BACKWARD -> conj

let dagify n sym f =
  let a = Array.init n (sym.osym.store n f)
  in Exprdag.make (List.flatten (Array.to_list a))
  
let no_twiddle_gen_expr n sym dir =
  let sign = sign_of_dir dir in
  let fft = fftgen n sym (fun i -> (load_var (access_input i))) sign
  in dagify n sym fft

let twiddle_dit_gen_expr n sym_before_twiddle sym_after_twiddle dir =
  let sign = sign_of_dir dir in
  let conj_maybe = conj_of_dir dir in

  let loaded_input = freeze n (fun i -> (load_var (access_input i))) in
  let symmetrized_input = sym_before_twiddle.apply n loaded_input in

  (* The eta-expansion `let rec f i = g i' instead of `let rec f = g'
     is a way to get around ocaml's limitations in handling let rec *)
  let rec twiddle_expression i =
    freeze n 
      (fun i ->
	(let (t, _, _) = twiddle_policy () in t n i twiddle_expression))
      i in

  let input_by_twiddle = 
    freeze n (fun i ->
      if (i == 0) then
	symmetrized_input 0
      else
	conj_maybe (twiddle_expression i) @* (symmetrized_input i))

  in let fft = fftgen n sym_after_twiddle input_by_twiddle sign
  in dagify n sym_after_twiddle fft

let twiddle_dif_gen_expr n sym_input sym_after_twiddle dir =
  let sign = sign_of_dir dir in
  let conj_maybe = conj_of_dir dir in

  let loaded_input = freeze n (fun i -> (load_var (access_input i))) in

  let fft = fftgen n sym_input loaded_input sign in

  (* The eta-expansion `let rec f i = g i' instead of `let rec f = g'
     is a way to get around ocaml's limitations in handling let rec *)

  let rec twiddle_expression i =
    freeze n 
      (fun i ->
	(let (t, _, _) = twiddle_policy () in t n i twiddle_expression))
      i in

  let fft_by_twiddle = 
    freeze n (fun i ->
      if (i == 0) then
	fft 0
      else
	conj_maybe (twiddle_expression i) @*
	   (fft i))

  in dagify n sym_after_twiddle fft_by_twiddle


