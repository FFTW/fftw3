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
(* $Id: complex.ml,v 1.5 2003-03-15 20:29:42 stevenj Exp $ *)

(* abstraction layer for complex operations *)
open Littlesimp
open Expr

(* type of complex expressions *)
type expr = CE of Expr.expr * Expr.expr

let two = CE (makeNum Number.two, makeNum Number.zero)
let one = CE (makeNum Number.one, makeNum Number.zero)
let i = CE (makeNum Number.zero, makeNum Number.one)
let zero = CE (makeNum Number.zero, makeNum Number.zero)
let make (r, i) = CE (r, i)

let uminus (CE (a, b)) =  CE (makeUminus a, makeUminus b)

let inverse_int n = CE (makeNum (Number.div Number.one (Number.of_int n)),
			makeNum Number.zero)

let nan x = CE (NaN x, makeNum Number.zero)

let half = inverse_int 2

let times_4_2 (CE (a, b)) (CE (c, d)) = 
  CE (makePlus [makeTimes (a, c); makeUminus (makeTimes (b, d))],
      makePlus [makeTimes (a, d); makeTimes (b, c)])

(* fma-rich multiplications of complex numbers *)
(*
  The complex multiplication (a+ib)(c+id) can be viewed as
  a matrix multiplication
 
     / a  -b \  / c \ 
     |       |  |   |
     \ b   a /  \ d /

  Assuming a^2 + b^2 = 1, we have

     / a  -b \  
     |       |  = U L U
     \ b   a /  

            / 1  (a-1)/b \            / 1  0 \
  where U = |            |   and  L = |      |
            \ 0    1     /            \ b  1 /

  (A rotation is the product of three shears.)

  We assume that a and b are constants so that U and L can be computed
  at compile time.  Applied blindly, however, this formula produces
  too many constants, because if (a, b) appears in the FFT algorithm,
  then (+/- a, +/- b) and (+/- b, +/- a) are also likely to appear,
  but each combination leads to a different value of (a-1)/b which
  needs to be stored somewhere.  Consequently, we use other simple
  identities to apply the formula only in the case a > 0, |a| < |b|
 *)
let rec times_fma (CE (a, b)) (CE (c, d)) =
  let abs a = if Number.negative a then Number.negate a else a in
  let sq a = Number.mul a a in
  match (a, b) with
    ((Num a), (Num b)) ->
      if Number.is_one (Number.add (sq a) (sq b)) &&
	not (Number.is_zero b) && not (Number.is_zero a) then
	begin
	(* formula is applicable *)
	  if Number.greater (abs b) (abs a) then
	    (* (a + ib) (c + id) = - (b - ia) (d - ic) *)
	    uminus 
	      (times_fma 
		 (CE (makeNum b, makeNum (Number.negate a)))
		 (CE (d, makeUminus c)))
	  else if Number.negative a then
	    (* (a+ib)(c+id) = -(-a-ib)(c+id) *)
	    uminus (times_fma (CE ((makeNum (Number.negate a)),
				   (makeNum (Number.negate b))))
		      (CE (c, d)))	
	  else
	    let am1ob = Number.div (Number.sub a Number.one) b in
	    let c = makePlus [c; makeTimes (makeNum am1ob, d)] in
	    let d = makePlus [d; makeTimes (makeNum b, c)] in
	    let c = makePlus [c; makeTimes (makeNum am1ob, d)] in
	    CE (c, d)
	end
      else
	(* unapplicable *)
	times_4_2 (CE (Num a, Num b)) (CE (c, d))
  | _ ->
      match (c, d) with
	((Num _), (Num _)) ->
	  times_fma (CE (c, d)) (CE (a, b))
      |	_ -> times_4_2 (CE (a, b)) (CE (c, d))
      
let times = times_4_2
(* let times = times_fma *)

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
let imag (CE (a, b)) = CE (b, makeNum Number.zero)
let conj (CE (a, b)) = CE (a, makeUminus b)

    
(* abstraction of sum_{i=0}^{n-1} *)
let sigma a b f = plus (List.map f (Util.interval a b))

(* store and assignment operations *)
let store_real v (CE (a, b)) = Expr.Store (v, a)
let store_imag v (CE (a, b)) = Expr.Store (v, b)
let store (vr, vi) x = (store_real vr x, store_imag vi x)

let assign_real v (CE (a, b)) = Expr.Assign (v, a)
let assign_imag v (CE (a, b)) = Expr.Assign (v, b)
let assign (vr, vi) x = (assign_real vr x, assign_imag vi x)


(*****************************************************************
 * special formulas for complex numbers w where |w| = 1 
 *****************************************************************)
(* (a + bi)^2 = (1 - 2b^2) + 2abi *)
let wsquare (CE (a, b)) =
  let twob = makeTimes (makeNum Number.two, b)
  in let twobsq = makeTimes (twob, b)
  and twoab = makeTimes (twob, a) in
  CE (makePlus [makeNum Number.one; makeUminus (twobsq)], twoab)

(* 
   ``reflection'' formulas
   
   w^{x+y} +- w^{x-y} = w^x (w^y +- w^{-y})

   Compute w^{x+y} from w^x and w^y
 *)

let wreflectc wxmy wx wy = 
   plus [uminus wxmy; times wx (plus [wy; (conj wy)])]

let wreflects wxmy wx wy = 
   plus [wxmy; times wx (plus [wy; uminus (conj wy)])]
  

(************************
   shortcuts 
 ************************)
let (@*) = times
let (@+) a b = plus [a; b]
let (@-) a b = plus [a; uminus b]

(* type of complex signals *)
type signal = int -> expr

(* make a finite signal infinite *)
let infinite n signal i = if ((0 <= i) && (i < n)) then signal i else zero

let hermitian n a =
  Util.array n (fun i ->
    if (i = 0) then real (a 0)
    else if (i < n - i)  then (a i)
    else if (i > n - i)  then conj (a (n - i))
    else real (a i))

