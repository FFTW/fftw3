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

(*
 * Phil Wadler has many well written papers about monads.  See
 * http://cm.bell-labs.com/cm/cs/who/wadler/ 
 *)

(* vanilla state monad *)
let unitM x = fun s -> (x, s)

let (>>=) = fun m k -> 
  fun s ->
    let (a', s') = m s
    in let (a'', s'') = k a' s'
    in (a'', s'')

let (>>) = fun m k ->
  m >>= fun _ -> k

let rec mapM f = function
  | [] -> unitM []
  | x::xs ->
      f x >>= fun x' ->
	mapM f xs >>= fun xs' ->
	  unitM (x'::xs')

let mapPairM f (a,b) = 
  f a >>= fun a' ->
    f b >>= fun b' ->
      unitM (a',b')

let mapTripleM f (a,b,c) = 
  f a >>= fun a' ->
    f b >>= fun b' ->
      f c >>= fun c' ->
      unitM (a',b',c')

let rec iterM fM = function
  | []    -> unitM ()
  | x::xs -> fM x >>= fun _ -> iterM fM xs

let ignoreM fM = fM >> unitM ()

let runM m x initial_state =
  let (a, _) = m x initial_state
  in a

let fetchStateM =
  fun s -> s, s

let storeStateM newState =
  fun _ -> (), newState

let consM x xs = unitM (x::xs)
