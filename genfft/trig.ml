(*
 * Copyright (c) 1997-1999 Massachusetts Institute of Technology
 * Copyright (c) 2000 Matteo Frigo
 * Copyright (c) 2000 Steven G. Johnson
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
(* $Id: trig.ml,v 1.2 2002-06-06 22:03:17 athena Exp $ *)

(* trigonometric transforms *)
open Util

(* DFT of real input *)
let rdft sign n input =
  Fft.dft sign n (Complex.real @@ input)

(* DFT of hermitian input *)
let hdft sign n input =
  Fft.dft sign n (Genutil.hermitian_array_c n input)

(* Discrete Hartley Transform *)
let dht sign n input =
  let f = Fft.dft sign n (Complex.real @@ input) in
  (fun i ->
    Complex.plus [Complex.real (f i); Complex.imag (f i)])

let dftI n input =
  let twon = 2 * n in
  let input' = array twon (fun i ->
    if (i == 0) then
      Complex.times Complex.two (input i)
    else if (i < n) then
      input i
    else if (i > twon - n) then
      Complex.conj (input (twon - i))
    else
      Complex.zero)
  in
  Fft.dft 1 twon input'

let dftII n input =
  let twon = 2 * n 
  and fourn = 4 * n in
  let input' = array fourn (fun i ->
    if (i mod 2) == 0
	then Complex.zero
    else 
      let i = (i - 1) / 2 in
      if (i < n) then
	(input i)
      else (* twon > i >= n *)
	Complex.conj (input (twon - 1 - i)))
  in
  Fft.dft 1 fourn input'

let dftIII n input =
  let fourn = 4 * n in
  let input' = array fourn (fun i ->
    if (i == 0) then
      Complex.times Complex.two (input i)
    else if (i < n) then
      input i
    else if (i > fourn - n) then
      Complex.conj (input (fourn - i))
    else
      Complex.zero)
  in
  let dft = Fft.dft 1 fourn input'
  in fun k -> dft (2 * k + 1)

let dftIV n input =
  let fourn = 4 * n
  and eightn = 8 * n in
  let input' = array eightn (fun i ->
    if (i mod 2) == 0
	then Complex.zero
    else 
      let i = (i - 1) / 2 in
      if (i < n) then
	input i
      else if (i >= fourn - n) then
	Complex.conj (input (fourn - 1 - i))
      else
	Complex.zero)
  in
  let dft = Fft.dft 1 eightn input'
  in fun k -> dft (2 * k + 1)


let make_dct dft =
  fun n input ->
    dft n (Complex.real (* @@ (Complex.times Complex.half) *) @@ input)

(*
 * DCT-I:  y[k] = sum x[j] cos(pi * j * k / n)
 *)
let dctI = make_dct dftI

(*
 * DCT-II:  y[k] = sum x[j] cos(pi * (j + 1/2) * k / n)
 *)
let dctII = make_dct dftII

(*
 * DCT-III:  y[k] = sum x[j] cos(pi * j * (k + 1/2) / n)
 *)
let dctIII = make_dct dftIII

(*
 * DCT-IV  y[k] = sum x[j] cos(pi * (j + 1/2) * (k + 1/2) / n)
 *)
let dctIV = make_dct dftIV



(* DST-x input := DFT-x (input / 2i) *)
let make_dst dft =
  fun n input ->
    Complex.real @@ 
    (dft n (Complex.uminus @@
	    (Complex.times Complex.i) @@
	    (Complex.times Complex.half) @@ 
	    Complex.real @@ 
	    input))

(*
 * DST-I:  y[k] = sum x[j] sin(pi * j * k / n)
 *)
let dstI = make_dst dftI

(*
 * DST-II:  y[k] = sum x[j] sin(pi * (j + 1/2) * k / n)
 *)
let dstII = make_dst dftII

(*
 * DST-III:  y[k] = sum x[j] sin(pi * j * (k + 1/2) / n)
 *)
let dstIII = make_dst dftIII

(*
 * DST-IV  y[k] = sum x[j] sin(pi * (j + 1/2) * (k + 1/2) / n)
 *)
let dstIV = make_dst dftIV

