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

(* This module declares basics data-types for virtual simd instructions
 * and defines some frequently used functions on them.
 *)

open List
open Util
open Number
open Variable
open VFpBasics


type vintreg = V_IntReg of int		(* VIRTUAL INTEGER REGISTER *********)
type vsimdreg = V_SimdReg of int	(* VIRTUAL SIMD REGISTER ************)

let next_vintreg  (V_IntReg n)  = V_IntReg (succ n)
let next_vsimdreg (V_SimdReg n) = V_SimdReg (succ n)

let makeNewVsimdreg = 
  let vsimdreg_counter = ref 1000 in		(* avoid starting with 0 *)
  fun () ->
    let retval = !vsimdreg_counter in
      incr vsimdreg_counter;
      V_SimdReg retval

let makeNewVintreg = 
  let vintreg_counter = ref 1000 in		(* avoid starting with 0 *)
  fun () ->
    let retval = !vintreg_counter in
      incr vintreg_counter;
      V_IntReg retval

let makeNewVintreg2 () = 
  (makeNewVintreg (), makeNewVintreg ())

let makeNewVintreg3 () =
  (makeNewVintreg (), makeNewVintreg (), makeNewVintreg ())

module VIntRegMap = Map.Make(struct type t = vintreg let compare = compare end)

let vintregmap_find k m = try Some(VIntRegMap.find k m) with Not_found -> None
let vintregmap_findE k m = try VIntRegMap.find k m with Not_found -> []
let vintregmap_addE k v m = VIntRegMap.add k (v::(vintregmap_findE k m)) m
let vintregmap_cutE k m = VIntRegMap.add k (tl (VIntRegMap.find k m)) m

module VSimdRegSet = Set.Make(struct type t = vsimdreg let compare = compare end)

module VSimdRegMap = Map.Make(struct type t = vsimdreg let compare = compare end)
let vsimdregmap_find k m = try Some(VSimdRegMap.find k m) with Not_found -> None

let vsimdregmap_findE k m = try VSimdRegMap.find k m with Not_found -> []
let vsimdregmap_addE k v m = VSimdRegMap.add k (v::(vsimdregmap_findE k m)) m
let vsimdregmap_addE' value key map = vsimdregmap_addE key value map
let vsimdregmap_cutE k m = VSimdRegMap.add k (tl (VSimdRegMap.find k m)) m

(****************************************************************************)

type vsimdpos = V_Lo | V_Hi | V_LoHi	(* POSITION WITHIN A SIMD QWORD *****)
type realimag = RealPart | ImagPart	(* COMPONENT OF A COMPLEX NUMBER ****)

(****************************************************************************)

type vconst =
  | V_NumberPairConst of float * float
  | V_NamedConst of string
  | V_ChsConst of vsimdpos

module VConstMap = Map.Make(struct type t = vconst let compare = compare end)

(****************************************************************************)

type vsimdinstroperand = 		 (* VIRTUAL SIMD INSTR OPERAND ******)
  | V_SimdTmp of int			 (*   Simd Temporary Variable  [id] *)
  | V_SimdDVar of vfpaccess * array * int (*   Simd DWord Var      [arr,idx] *)
  | V_SimdQVar of array * int		 (*   Simd QWord Var      [arr,idx] *)
  | V_SimdNConst of string 		 (*   Named Const	 [name_str] *)
  | V_SimdNumConst of float * float  	 (*   Numeric Constant	[c_lo,c_hi] *)

module VSimdInstrOperandSet = Set.Make(struct 
					 type t = vsimdinstroperand 
					 let compare = compare 
				       end)

type vsimdrawvar = 			(* RAW SIMD VARIABLE (w/o Array) ****)
  | V_SimdRawQVar of int
  | V_SimdRawDVar of vfpaccess * int

module VSimdRawVarSet = Set.Make(struct 
				   type t = vsimdrawvar 
				   let compare = compare 
				 end)

(* Returns true if assignment of a clobbers variable b.  Do not mix
 * SimdQVars and SimdDVar (-> possible undetected aliasing conflict). *)
(*
function should work, but is no longer in use.

let vsimdinstroperand_clobbers a b = match (a,b) with
  | V_SimdQVar(Output,k1),   V_SimdQVar(Input,k2) -> k1=k2
  | V_SimdDVar(p,Output,k1), V_SimdDVar(q,Input,k2) -> k1=k2 && p=q
  | _ -> false
*)

let vsimdinstroperand_is_temporary = function 
  | V_SimdTmp _ -> true 
  | _		-> false

let vsimdinstroperand_is_twiddle = function
  | V_SimdQVar(Twiddle,_)   -> true
  | V_SimdDVar(_,Twiddle,_) -> true
  | _ 			    -> false

(****************************************************************************)

type vsimdbinop =			(* SIMD BINARY OPERATION ************)
  | V_Add				(*   Parallel Addition		    *)
  | V_Sub				(*   Parallel Subtraction	    *)
  | V_SubR 				(*   Parallel Reversed Subtraction  *)
  | V_Mul				(*   Parallel Multiplication	    *)
  | V_PPAcc				(*   Intraoperand PosPos Accumulate *)
  | V_NNAcc				(*   Intraoperand NegNeg Accumulate *)
  | V_NPAcc				(*   Intraoperand NegPos Accumulate *)
  | V_UnpckLo				(*   Join Lo Parts 		    *)
  | V_UnpckHi				(*   Join Hi Parts		    *)

type vsimdunaryop =			(* SIMD UNARY OPERATION *************)
  | V_Id				(*   Copy Contents		    *)
  | V_Swap				(*   Flip Lo/Hi Part of 2-way Reg   *)
  | V_Chs of vsimdpos 			(*   Negate Part of Simd Register   *)
  | V_MulConst of number * number	(*   Multiply with 2-way Constant   *)

let eq_vsimdunaryop a b = match (a,b) with
  | (V_Id, V_Id) -> true
  | (V_Swap, V_Swap) -> true
  | (V_Chs p1, V_Chs p2) -> p1=p2
  | (V_MulConst(n,m), V_MulConst(n',m')) -> 
      Number.equal n n' && Number.equal m m'
  | _ -> false

let vsimdbinopIsParallel = function	(* inter-operand parallel binary-op *)
  | V_Add     -> true
  | V_Sub     -> true
  | V_SubR    -> true
  | V_Mul     -> true
  | V_UnpckLo -> false
  | V_UnpckHi -> false
  | V_PPAcc   -> false
  | V_NNAcc   -> false
  | V_NPAcc   -> false

(****************************************************************************)

type vsimdinstr = 			(* VIRTUAL SIMD INSTRUCTION *********)
  | V_SimdLoadQ of 			(*   Load SIMD Quadword:	    *)
	array * 			(*	= source array (s_array)    *)
	int * 				(*      = src element index (s_idx) *)
	vsimdreg			(*	= dst simd register (d_reg) *)
  | V_SimdStoreQ of 			(*   Store SIMD Quadword:	    *)
	vsimdreg *			(*	= src simd register (s_reg) *)
	array * 			(*	= dst array (d_array)	    *)
	int				(*	= dst element index (d_idx) *)
  | V_SimdLoadD of 			(*   Load Real Array Element:	    *)
	vfpaccess *			(*	= access information (s)    *)
	array * 			(*	= source array (s_array)    *)
	int * 				(*      = src element index (s_idx) *)
	vsimdpos * 			(*	= dst simd register pos     *)
	vsimdreg			(*	= dst simd register (d_reg) *)
  | V_SimdStoreD of 			(*   Store Real Array Element:	    *)
	vsimdpos * 			(*	= src simd register pos     *)
	vsimdreg *			(*	= src simd register (s_reg) *)
	vfpaccess *			(*	= access information (d)    *)
	array * 			(*	= dst array (d_array)	    *)
	int  				(*      = dst element index (d_idx) *)
  | V_SimdUnaryOp of 			(*   Do 2-Way Unary Operation:      *)
	vsimdunaryop *			(*	= operation id (unary_op)   *)
	vsimdreg *			(*	= source register (s_reg)   *)
	vsimdreg			(*	= dest register (d_reg)     *)
  | V_SimdBinOp of 			(*   Do 2-Way Binary Operation:     *)
	vsimdbinop *			(*	= operation id (binary_op)  *)
	vsimdreg * 			(*	= source reg #1 (s1_reg)    *)
	vsimdreg *			(*	= source reg #2 (s2_reg)    *)
	vsimdreg			(*	= dest register (d_reg)     *)


(* Output list does not contain any duplicates. *)
let vsimdinstrToSrcregs = function
  | V_SimdLoadD _	    -> []
  | V_SimdLoadQ _	    -> []
  | V_SimdStoreD(_,s,_,_,_) -> [s]
  | V_SimdStoreQ(s,_,_)     -> [s]
  | V_SimdBinOp(_,s1,s2,_)  -> if s1<>s2 then [s1;s2] else [s1]
  | V_SimdUnaryOp(_,s,_)    -> [s]

let vsimdinstrToDstreg = function
  | V_SimdStoreD _ 	    -> None
  | V_SimdStoreQ _          -> None
  | V_SimdLoadD(_,_,_,_,d)  -> Some d
  | V_SimdLoadQ(_,_,d)      -> Some d
  | V_SimdBinOp(_,_,_,d)    -> Some d
  | V_SimdUnaryOp(_,_,d)    -> Some d

let vsimdinstrToReads  instr = vsimdinstrToSrcregs instr
let vsimdinstrToWrites instr = optionToList (vsimdinstrToDstreg instr) 
let vsimdinstrToReadswritespair i = (vsimdinstrToReads i,vsimdinstrToWrites i)

(****************************************************************************)

let vsimdposToSwapped = function
  | V_Lo   -> V_Hi
  | V_Hi   -> V_Lo
  | V_LoHi -> V_LoHi

let vsimdposToSrcoperand = function
  | V_Lo   -> V_SimdNConst "chsl"
  | V_Hi   -> V_SimdNConst "chsh"
  | V_LoHi -> V_SimdNConst "chslh"

let vsimdunaryopToSrcoperands = function
  | V_MulConst(n,m) -> [V_SimdNumConst(Number.to_float n, Number.to_float m)]
  | V_Chs p	    -> [vsimdposToSrcoperand p]
  | V_Id	    -> []
  | V_Swap	    -> []

let vsimdinstrToSrcoperands = function
  | V_SimdLoadD(reim,arr,el,_,_)      -> [V_SimdDVar(reim,arr,el)]
  | V_SimdLoadQ(arr,el,_) 	      -> [V_SimdQVar(arr,el)]
  | V_SimdStoreD(_,V_SimdReg s,_,_,_) -> [V_SimdTmp s]
  | V_SimdStoreQ(V_SimdReg s,_,_)     -> [V_SimdTmp s]
  | V_SimdUnaryOp(op,V_SimdReg s,_)   -> (V_SimdTmp s)::
					   (vsimdunaryopToSrcoperands op)
  | V_SimdBinOp(_,V_SimdReg s1,V_SimdReg s2,_) -> [V_SimdTmp s1; V_SimdTmp s2]


let vsimdinstrToSrcrawvars = function
  | V_SimdLoadD(access,_,idx,_,_) -> [V_SimdRawDVar(access,idx)]
  | V_SimdLoadQ(_,idx,_)	  -> [V_SimdRawQVar idx]
  | V_SimdStoreD _		  -> []
  | V_SimdStoreQ _		  -> []
  | V_SimdUnaryOp _		  -> []
  | V_SimdBinOp _		  -> []

let vsimdinstrToDstoperands = function
  | V_SimdLoadD(_,_,_,_,V_SimdReg d) -> [V_SimdTmp d]
  | V_SimdLoadQ(_,_,V_SimdReg d)     -> [V_SimdTmp d]
  | V_SimdStoreD(_,_,access,arr,el)  -> [V_SimdDVar(access,arr,el)]
  | V_SimdStoreQ(_,arr,el) 	     -> [V_SimdQVar(arr,el)]
  | V_SimdBinOp(_,_,_,V_SimdReg d)   -> [V_SimdTmp d]
  | V_SimdUnaryOp(_,_,V_SimdReg d)   -> [V_SimdTmp d]

let vsimdinstrIsLoad = function
  | V_SimdLoadD _    -> true
  | V_SimdLoadQ _    -> true
  | V_SimdStoreD _   -> false
  | V_SimdStoreQ _   -> false
  | V_SimdBinOp _    -> false
  | V_SimdUnaryOp _  -> false

let vsimdinstrIsStore = function
  | V_SimdLoadD _    -> false
  | V_SimdLoadQ _    -> false
  | V_SimdStoreD _   -> true
  | V_SimdStoreQ _   -> true
  | V_SimdBinOp _    -> false
  | V_SimdUnaryOp _  -> false

(****************************************************************************)

let scalarvfpunaryopToVsimdunaryop op pos = match (pos,op) with
  | (_, V_FPId)	   	     -> V_Id
  | (p, V_FPNegate)          -> V_Chs p
  | (V_Lo, V_FPMulConst n)   -> V_MulConst(n, Number.one)
  | (V_Hi, V_FPMulConst n)   -> V_MulConst(Number.one, n)
  | (V_LoHi, V_FPMulConst n) -> V_MulConst(n, n)

let vfpbinopToVsimdbinop = function
  | V_FPAdd -> V_Add
  | V_FPSub -> V_Sub
  | V_FPMul -> V_Mul

