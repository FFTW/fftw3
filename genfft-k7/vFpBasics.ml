(*
 * Copyright (c) 2000-2001 Stefan Kral
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

(* This module declares basic data types for virtual scalar floating-point
 * instructions and common operations on them.  *)

open List
open Util
open Number				(* Arbitrary Precision Number Type  *)
open Variable				(* Def.s of Variables and Arrays    *)

type vfpcomplexarrayformat = 
  | SplitFormat 
  | InterleavedFormat

type vfparraytype = 
  | RealArray 
  | ComplexArray of vfpcomplexarrayformat

let vfparraytypeIsReal = function
  | RealArray 	   -> true
  | ComplexArray _ -> false

let vfparraytypeIsComplex = function
  | RealArray 	   -> false
  | ComplexArray _ -> true

(****************************************************************************)

type vfpreg = V_FPReg of int		(* VIRTUAL SCALAR FP REGISTER *******)

type vfpsummand = V_FPPlus of vfpreg | V_FPMinus of vfpreg

(* makeNewVfpreg () creates a new vfpreg. It uses Variable.make_temporary   *
 * so that the indices of vfpregs and temporary variables (that have been   *
 * created in the generator) are ``compatible''.			    *)
let makeNewVfpreg () =
  match Variable.make_temporary () with
    | Temporary t -> V_FPReg t
    | _ -> failwith "VFPBasics.makeNewVfpreg: Unexpected failure!"

(* Sets and Maps with key type vfpreg *)
module VFPRegSet = Set.Make(struct type t = vfpreg let compare = compare end)
module VFPRegMap = Map.Make(struct type t = vfpreg let compare = compare end)

let vfpregmap_find k m = try Some(VFPRegMap.find k m) with Not_found -> None

(* The function vfpregmap_addE inserts a (key,value) pair into a multimap   *
 * m. All bindings bindings of key are stored in an unsorted list. 	    *)
let vfpregmap_findE k m = try VFPRegMap.find k m with Not_found -> []
let vfpregmap_addE k v m = VFPRegMap.add k (v::(vfpregmap_findE k m)) m

(****************************************************************************)

type vfpaccess =
  | V_FPRealOfComplex
  | V_FPImagOfComplex
  | V_FPReal

type vfpunaryop =
  | V_FPMulConst of number
  | V_FPId
  | V_FPNegate

type vfpbinop = 
  | V_FPAdd
  | V_FPSub
  | V_FPMul

type vfpinstr = 			(* VIRTUAL SCALAR FP INSTRUCTION ****)
  | V_FPLoad of 
	vfpaccess *
	array *
	int *
	vfpreg
  | V_FPStore of
	vfpreg *
	vfpaccess *
	array *
	int
  | V_FPUnaryOp of
	vfpunaryop *
	vfpreg *
	vfpreg
  | V_FPBinOp of
	vfpbinop *
	vfpreg *
	vfpreg *
	vfpreg
  | V_FPAddL of
	vfpsummand list *
	vfpreg

(****************************************************************************)

let vfpbinopIsAddorsub = function
  | V_FPAdd -> true
  | V_FPSub -> true
  | V_FPMul -> false

let vfpunaryopToNegated = function
  | V_FPId         -> V_FPNegate
  | V_FPNegate     -> V_FPId
  | V_FPMulConst n -> V_FPMulConst (Number.negate n)

let vfpsummandToVfpreg = function
  | V_FPPlus  x -> x
  | V_FPMinus x -> x

let vfpsummandIsPositive = function
  | V_FPPlus _  -> true
  | V_FPMinus _ -> false

let vfpsummandIsNegative = function
  | V_FPPlus _  -> false
  | V_FPMinus _ -> true

let vfpsummandToNegated = function
  | V_FPPlus x  -> V_FPMinus x
  | V_FPMinus x -> V_FPPlus x

(****************************************************************************)

(* vfpinstrToSrcregs maps a vfpinstruction to its source register operands. *
 * The output list may contain duplicates.				    *)
let vfpinstrToSrcregs = function
  | V_FPLoad _ 		 -> []
  | V_FPStore(s,_,_,_)   -> [s]
  | V_FPUnaryOp(_,s,_)   -> [s]
  | V_FPBinOp(_,s1,s2,_) -> [s1;s2]
  | V_FPAddL(srcs,_)     -> map vfpsummandToVfpreg srcs
  
(* vfpinstrToDstreg maps a vfpinstr to its destination register operand. If *
 * an instruction does not have a dest register-operand, None is returned.  *)
let vfpinstrToDstreg = function
  | V_FPStore _	       -> None
  | V_FPLoad(_,_,_,d)  -> Some d
  | V_FPUnaryOp(_,_,d) -> Some d
  | V_FPBinOp(_,_,_,d) -> Some d
  | V_FPAddL(_,d)      -> Some d

let vfpinstrToVfpregs instr = 
  optionToListAndConcat (vfpinstrToSrcregs instr) (vfpinstrToDstreg instr)

let vfpinstrIsLoad = function
  | V_FPLoad _ -> true
  | _ -> false

let vfpinstrIsStore = function
  | V_FPStore _ -> true
  | _ -> false

let vfpinstrIsLoadOrStore i = vfpinstrIsLoad i || vfpinstrIsStore i

let vfpinstrToAddsubcount = function
  | V_FPLoad _          -> 0
  | V_FPStore _         -> 0
  | V_FPUnaryOp _       -> 0
  | V_FPBinOp(op,_,_,_) -> if vfpbinopIsAddorsub op then 1 else 0
  | V_FPAddL(xs,_)      -> length xs

let vfpinstrIsBinMul = function
  | V_FPBinOp(V_FPMul,_,_,_) -> true
  | _ -> false

let vfpinstrIsUnaryOp = function
  | V_FPUnaryOp _ -> true
  | _ -> false


(****************************************************************************)

let addlistHelper2 dst = function
  | (V_FPPlus  x, V_FPPlus y)  -> (V_FPBinOp(V_FPAdd,x,y,dst), V_FPPlus dst)
  | (V_FPPlus  x, V_FPMinus y) -> (V_FPBinOp(V_FPSub,x,y,dst), V_FPPlus dst)
  | (V_FPMinus x, V_FPPlus y)  -> (V_FPBinOp(V_FPSub,y,x,dst), V_FPPlus dst)
  | (V_FPMinus x, V_FPMinus y) -> (V_FPBinOp(V_FPAdd,x,y,dst), V_FPMinus dst)

let addlistHelper3' dst tmp (x',y',z') =
  let (i1, tmp') = addlistHelper2 tmp (x',y') in
  let (i2, dst') = addlistHelper2 dst (tmp',z') in
    (i1,i2,dst')

let addlistHelper3 dst triple =
  addlistHelper3' dst (makeNewVfpreg ()) triple


