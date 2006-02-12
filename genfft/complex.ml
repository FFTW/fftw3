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
(* $Id: complex.ml,v 1.9 2006-02-12 23:34:12 athena Exp $ *)

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

let inverse_int_sqrt n = 
  CE (makeNum (Number.div Number.one (Number.sqrt (Number.of_int n))),
      makeNum Number.zero)
let int_sqrt n = 
  CE (makeNum (Number.sqrt (Number.of_int n)),
      makeNum Number.zero)

let nan x = CE (NaN x, makeNum Number.zero)

let half = inverse_int 2

let times (CE (a, b)) (CE (c, d)) = 
  CE (makePlus [makeTimes (a, c); makeUminus (makeTimes (b, d))],
      makePlus [makeTimes (a, d); makeTimes (b, c)])

let ctimes (CE (a, _)) (CE (c, _)) = 
  CE (CTimes (a, c), makeNum Number.zero)

let ctimesj (CE (a, _)) (CE (c, _)) = 
  CE (CTimesJ (a, c), makeNum Number.zero)
      
(* complex exponential (of root of unity); returns exp(2*pi*i/n * m) *)
let exp n i =
  let (c, s) = Number.cexp n i
  in CE (makeNum c, makeNum s)

(* various trig functions evaluated at (2*pi*i/n * m) *)
let sec n m =
  let (c, s) = Number.cexp n m
  in CE (makeNum (Number.div Number.one c), makeNum Number.zero)
let csc n m =
  let (c, s) = Number.cexp n m
  in CE (makeNum (Number.div Number.one s), makeNum Number.zero)
let tan n m =
  let (c, s) = Number.cexp n m
  in CE (makeNum (Number.div s c), makeNum Number.zero)
let cot n m =
  let (c, s) = Number.cexp n m
  in CE (makeNum (Number.div c s), makeNum Number.zero)
    
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
let iimag (CE (a, b)) = CE (makeNum Number.zero, b)
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

let antihermitian n a =
  Util.array n (fun i ->
    if (i = 0) then iimag (a 0)
    else if (i < n - i)  then (a i)
    else if (i > n - i)  then uminus (conj (a (n - i)))
    else iimag (a i))
