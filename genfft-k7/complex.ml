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

(* $Id: complex.ml,v 1.1 2002-06-14 10:56:15 athena Exp $ *)

(* abstraction layer for complex operations *)

(* type of complex expressions *)
open Exprdag
open Exprdag.LittleSimplifier

type expr = CE of node * node

let one  = CE (makeNum Number.one,  makeNum Number.zero)
let zero = CE (makeNum Number.zero, makeNum Number.zero)

let inverse_int n = CE (makeNum (Number.div Number.one 
			       (Number.of_int n)),
			makeNum Number.zero)

let times_4_2 (CE (a, b)) (CE (c, d)) = 
  CE (makePlus [makeTimes (a, c); makeUminus (makeTimes (b, d))],
      makePlus [makeTimes (a, d); makeTimes (b, c)])

let simple = function
    Num a -> Number.is_zero a or Number.is_one a or Number.is_mone a
  | _ -> false

let rec times_3_3 (CE (a, b)) (CE (c, d)) = 
  (* refuse to do the 3-3 algorithm if a=1, i, -i, -1, etc. *)
  if simple a or simple b or simple c or simple d then 
    times_4_2 (CE (c, d)) (CE (a, b))
  else match a with
    Num _ ->
      let amb = makePlus [a; makeUminus b]
      and cpd = makePlus [c; d]
      and apb = makePlus [a; b]
      in let apbc = makeTimes (apb, c)
      and bcpd = makeTimes (b, cpd)
      and ambd = makeTimes (amb, d)
      in CE (makePlus [apbc; makeUminus bcpd],
	     makePlus [bcpd; ambd])
  | _ -> match c with
           Num _ -> times_3_3 (CE (c, d)) (CE (a, b))
         | _     -> times_4_2 (CE (a, b)) (CE (c, d))

let times a b = 
  if !Magic.times_3_3 then
    times_3_3 a b
  else
    times_4_2 a b

let uminus (CE(a,b)) = CE(makeUminus a, makeUminus b)

(* hack to swap real<->imaginary.  Used by hc2hc codelets *)
let swap_re_im (CE(r,i)) = CE(i,r)

(* complex exponential (of root of unity); returns exp(2*pi*i/n * m) *)
let exp n i =
  let (c, s) = Number.cexp n i
  in CE (makeNum c, makeNum s)
    
(* complex sum *)
let plus a =
  let rec unzip_complex = function
      [] -> ([], [])
    | ((CE (a, b)) :: s) ->
        let (r,i) = unzip_complex s
	in
	(a::r), (b::i) in
  let (c, d) = unzip_complex a in
  CE (makePlus c, makePlus d)

(* extract real/imaginary *)
let real (CE (a, b)) = CE (a, makeNum Number.zero)
let imag (CE (a, b)) = CE (makeNum Number.zero, b)
let conj (CE (a, b)) = CE (a, makeUminus b)
    
let abs_sqr (CE(a,b)) = makePlus [makeTimes(a,a); makeTimes(b,b)]

(*
 * special cases for complex numbers w where |w| = 1 
 *)
(* (a + bi)^2 = (2a^2 - 1) + 2abi *)
let wsquare (CE (a, b)) =
  let twoa = makeTimes (makeNum Number.two, a)
  in let twoasq = makeTimes (twoa, a)
  and twoab = makeTimes (twoa, b) in
  CE (makePlus [twoasq; makeUminus (makeNum Number.one)], twoab)

(*
 * compute w^n given w^{n-1}, w^{n-2}, and w, using the identity 
 * 
 * w^n + w^{n-2} = w^{n-1} (w + w^{-1}) = 2 w^{n-1} Re(w)
 *)
let wthree (CE (an1, bn1)) wn2 (CE (a, b)) =
  let twoa = makeTimes (makeNum Number.two, a)
  in let twoa_wn1 = CE (makeTimes (twoa, an1), 
			makeTimes (twoa, bn1))
  in plus [twoa_wn1; (uminus wn2)]

(* abstraction of sum_{i=0}^{n-1} *)
(* let sigma a b f = plus (Util.forall :: a b f) *)
let sigma a b f =
  let rec loop a = 
    if (a >= b) then []
    else (f a) :: (loop (a + 1))
  in plus (loop a)

(* complex variables *)
type variable = CV of Variable.variable * Variable.variable

let load_var (CV(vr,vi)) = CE (Load vr, Load vi)

let store_var  (CV(vr,vi)) (CE(xr,xi)) = [Store(vr,xr); Store(vi,xi)]
let store_real (CV(vr,vi)) (CE(xr,xi)) = [Store(vr,xr)]
let store_imag (CV(vr,vi)) (CE(xr,xi)) = [Store(vi,xi)]

let access what k =
  let (r, i) = what k
  in CV (r, i)

let access_input   = access Variable.access_input
let access_output  = access Variable.access_output
let access_twiddle = access Variable.access_twiddle

