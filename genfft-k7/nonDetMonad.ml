(*
 * Copyright (c) 2001 Stefan Kral
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


open Util

(****************************************************************************)

let identityM a state = Some(a,state)

let unitM a state cont = cont a state

let (>>=) = fun f1 f2 -> fun state cont -> 
  f1 state (fun a state' -> f2 a state' cont)

let (>>) = fun m k -> m >>= fun _ -> k	     (* may be dangerous (in ML) *)

let fetchStateM state cont = cont state state 
let storeStateM state _oldstate cont = cont () state

let catch1M exn handler state cont = 
  try cont () state with exn -> handler () state cont

let (|||) = fun f1 f2 -> 
  fun state cont ->
    match f1 () state cont with
      | None		-> f2 () state cont
      | Some(a',state')	-> Some(a',state')

let failM _state _cont = None

let runM f x state = mapOption fst (f x state identityM)
let runP f x state = optionIsSome (runM f x state)

let posAssertM = function true  -> unitM () | false -> failM
let negAssertM = function false -> unitM () | true  -> failM

(****************************************************************************)

let mapPairM f (a,b) = 
  f a >>= fun a' ->
    f b >>= fun b' ->
      unitM (a',b')

let mapTripleM f (a,b,c) = 
  f a >>= fun a' ->
    f b >>= fun b' ->
      f c >>= fun c' ->
	unitM (a',b',c')

let consM x xs = unitM (x::xs)

let rec mapM f = function
  | []    -> unitM []
  | x::xs -> f x >>= fun x' -> 
	       mapM f xs >>= fun xs' ->
		 unitM (x'::xs')

let optionToValueM = function 
  | None   -> failM 
  | Some x -> unitM x

let rec iterM f = function
  | []    -> unitM ()
  | x::xs -> f x >>= fun _ -> iterM f xs

let rec iterirevM f i = function
  | []    -> unitM ()
  | x::xs -> iterirevM f (succ i) xs >>= fun _ -> f i x

(* avoid creation of choicepoint for last member of a list *)
let rec memberM' el = function
  | []    -> unitM el
  | x::xs -> (fun _ -> unitM el) ||| (fun _ -> memberM' x xs)

let memberM = function
  | []    -> failM
  | x::xs -> memberM' x xs

(* auxiliary predicate to avoid creation of choicepoint for the last element *)
let rec selectM' el = function
  | []		 -> unitM (el,[])
  | x::xs as xxs -> (fun _ -> unitM (el,xxs)) |||
		    (fun _ -> selectM' x xs >>= fun (z,zs) -> unitM (z,el::zs))
let selectM = function
  | []    -> failM
  | x::xs -> selectM' x xs

let rec selectFirstM p = function
  | []    	   -> failM
  | x::xs when p x -> unitM (x,xs) 
  | x::xs 	   -> selectFirstM p xs >>= fun (z,zs) -> unitM (z,x::zs)

let deleteFirstM p xs = selectFirstM p xs >>= fun (_,zs) -> unitM zs

let rec permutationM = function
  | [] -> unitM []
  | xs -> selectM xs >>= fun (z,zs) -> permutationM zs >>= consM z

let rec forallM p = function
  | []    -> unitM ()
  | x::xs -> p x >>= fun _ -> forallM p xs 

let existsM p xs = memberM xs >>= p

(* auxiliary function to avoid creation of choicepoint *)
let rec betweenM' i0 i1 i =
  if i1 <= i then
    (fun _ -> unitM i0) ||| (fun _ -> betweenM' i1 (succ i1) i)
  else
    unitM i0

let betweenM i0 i =
  if i0 <= i then
    betweenM' i0 (succ i0) i
  else 
    failM

let disjM xs = memberM xs >>= fun f -> f ()

