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


(* This module include the declaration of the data types used within the
 * register allocator and some common functions on them.
 *)

open List
open Util
open VSimdBasics
open K7Basics

type initcodeinstr = 
  | AddIntOnDemandCode of vintreg * k7vinstr list
  | FixRegister of vintreg * K7Basics.k7rintreg

type riregfileentry =
  | IFree
  | IHolds of vintreg
  | IFixed of vintreg
  | ITmpHoldsProduct of vintreg * int		(* used for reusing EAs *)
  | IVarHoldsProduct of vintreg * vintreg * int

type rsregfileentry =
  | SFree
  | SHolds of vsimdreg

type vsregfileentry =
  | SFresh
  | SDead
  | SDying of k7rmmxreg
  | SIn of k7rmmxreg
  | SInAndOut of k7rmmxreg * int 
  | SInConst of k7rmmxreg * string
  | SOut of int
  | SConst of string
  | SOnDemand of k7vinstr list

type viregfileentry =
  | IFresh
  | IDead
  | IDying of k7rintreg
  | IIn of k7rintreg
  | IInAndOut of k7rintreg * int 
  | IInConst of k7rintreg * string
  | IOut of int
  | IConst of string
  | IOnDemand of k7vinstr list

let vsregfileentryIsFresh = function
  | SFresh -> true
  | _ -> false

let livevsregfileentryToK7rmmxreg = function
  | SIn x 	   -> Some x
  | SInAndOut(x,_) -> Some x
  | SInConst(x,_)  -> Some x
  | SDying _ 	   -> None
  | SOut _ 	   -> None
  | SConst _ 	   -> None
  | SFresh 	   -> None
  | SDead 	   -> failwith "livevsregfileentryToK7rmmxreg: dead register!"
  | SOnDemand _    -> failwith "livevsregfileentryToK7rmmxreg: ondemand reg!"

let vsregfileentryToK7memop = function		(* used only once! *)
  | SIn x 	   -> None
  | SInAndOut(x,_) -> None
  | SInConst(x,_)  -> None
  | SDying _ 	   -> None
  | SOut p 	   -> Some (K7_MStackCell(K7_MMXStack,p))
  | SConst c 	   -> Some (K7_MConst c)
  | SFresh 	   -> None
  | SDead 	   -> failwith "livevsregfileentryToK7rmmxreg: dead register!"
  | SOnDemand _    -> failwith "livevsregfileentryToK7rmmxreg: ondemand reg!"

let vsregfileentryToK7rmmxreg = function
  | SDying x -> Some x
  | x 	     -> livevsregfileentryToK7rmmxreg x

let liveviregfileentryToK7rintreg = function
  | IIn x 	   -> Some x
  | IInAndOut(x,_) -> Some x
  | IInConst(x,_)  -> Some x
  | IDying _ 	   -> None
  | IOut _ 	   -> None
  | IConst _	   -> None
  | IFresh	   -> None
  | IDead 	   -> failwith "liveviregfileentryToK7rintreg: dead register!"
  | IOnDemand _	   -> failwith "liveviregfileentryToK7rintreg: ondemand reg!"

let viregfileentryToK7rintreg = function
  | IDying x -> Some x
  | x 	     -> liveviregfileentryToK7rintreg x

let vsregfileentryToK7rmmxreg' e = optionToValue (vsregfileentryToK7rmmxreg e)
let viregfileentryToK7rintreg' e = optionToValue (viregfileentryToK7rintreg e)

let viregWantsIn viregfile vireg = match VIntRegMap.find vireg viregfile with
  | IOut _	-> true
  | IFresh	-> true
  | IConst _	-> true
  | IInAndOut _	-> false
  | IIn _	-> false
  | IInConst _	-> false
  | IDead	-> failwith "viregWantsIn: using dead register!"
  | IDying _	-> failwith "viregWantsIn: using dying register!"
  | IOnDemand _	-> failwith "viregWantsIn: encountered OnDemand register!"

let vsregWantsIn vsregfile vsreg = match VSimdRegMap.find vsreg vsregfile with
  | SOut _	-> true
  | SFresh	-> true
  | SConst _	-> true
  | SIn _	-> false
  | SInAndOut _	-> false
  | SInConst _	-> false
  | SDead	-> failwith "vsregWantsIn: using dead register!"
  | SDying _	-> failwith "vsregWantsIn: using dying register!"
  | SOnDemand _	-> failwith "vsregWantsIn: encountered OnDemand register!"


let touchSimdDstReg vsreg vsregfile = 
  match livevsregfileentryToK7rmmxreg (VSimdRegMap.find vsreg vsregfile) with
    | Some i -> VSimdRegMap.add vsreg (SIn i) vsregfile
    | None   -> failwith "touchSimdDstReg: register is unmapped!"

let touchIntDstReg vireg viregfile = 
  match liveviregfileentryToK7rintreg (VIntRegMap.find vireg viregfile) with
    | Some i -> VIntRegMap.add vireg (IIn i) viregfile
    | None   -> failwith "touchIntDstReg: register is unmapped!"

  

let vsregfileentryIsFresh = function
  | SFresh -> true
  | _ -> false

let viregfileentryIsFresh = function
  | IFresh -> true
  | _ -> false

let riregfileentryToVireg = function
  | IVarHoldsProduct(v,_,_) -> Some v
  | IHolds v 		    -> Some v
  | IFixed v 		    -> Some v
  | IFree		    -> None
  | ITmpHoldsProduct _      -> None

let riregfileentryIsFree x = optionIsNone (riregfileentryToVireg x)
