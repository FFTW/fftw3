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

(* This module includes definitions of data types and functions that are used
 * within the instruction scheduler. *)

open List
open Util
open K7Basics

type k7rflag = 				(* K7 FLAGS *************************)
  | K7RFlag_Zero			(*   Zero Flag			    *)

type k7rop =				(* K7 REAL OPERAND ******************)
  | K7ROp_MMXReg of k7rmmxreg		(*   MMX Register (SIMD)	    *)
  | K7ROp_IntReg of k7rintreg		(*   Integer Register		    *)
  | K7ROp_MemOp of k7memop		(*   Memory Operand		    *)
  | K7ROp_IP				(*   Instruction Pointer	    *)
  | K7ROp_Flag of k7rflag		(*   Flag (eg. zero)		    *)
  | K7ROp_MMXState			(*   Floating-Point Mode (x87/MMX)  *)
  | K7ROp_MemoryState			(*   for preserving Ld/St order     *)
  | K7ROp_Stack				(*   stack operations (push/pop)    *)

let zeroflag = K7ROp_Flag (K7RFlag_Zero)
let mmxstackcell i = K7ROp_MemOp(K7_MStackCell(K7_MMXStack, i))

module ResMap = Map.Make(struct type t = k7rop let compare = compare end)

let resmap_findE k m = try ResMap.find k m with Not_found -> []
let resmap_addE k v m = ResMap.add k (v::resmap_findE k m) m
let resmap_addE' value key = resmap_addE key value

let resmap_find k m = try Some (ResMap.find k m) with Not_found -> None


(* READING OF K7ROPS ********************************************************)

let raddrToRops = function
  | K7R_RID(base,_)           -> [K7ROp_IntReg base]
  | K7R_SID(index,_,_)        -> [K7ROp_IntReg index]
  | K7R_RISID(base,index,_,_) -> [K7ROp_IntReg base; K7ROp_IntReg index]

let intuopToSrcrops sd = function
  | K7_IPush  	    -> [K7ROp_IntReg k7rintreg_stackpointer; sd]
  | K7_IPop  	    -> [K7ROp_IntReg k7rintreg_stackpointer]
  | K7_IClear 	    -> []
  | K7_ILoadValue _ -> []
  | K7_INegate 	    -> [sd]
  | K7_IInc 	    -> [sd]
  | K7_IDec 	    -> [sd]
  | K7_IAddImm _    -> [sd]
  | K7_ISubImm _    -> [sd]
  | K7_IShlImm _    -> [sd]
  | K7_IShrImm _    -> [sd]

let intcpyuopToSrcrops = function
  | K7_ICopy	 -> []
  | K7_IMulImm _ -> []

let branchconditionToSrcrops = function
  | K7_BCond_NotZero	 -> [zeroflag]
  | K7_BCond_Zero	 -> [zeroflag]
  | K7_BCond_GreaterZero -> [zeroflag]
  | K7_BCond_EqualZero	 -> [zeroflag]

let simduopToSrcrops sd = function
  | K7_FPChs _ 	    -> [sd]
  | K7_FPMulConst _ -> [sd]

let simdcpyuopToSrcrops = function
  | K7_FPId   -> []
  | K7_FPSwap -> []

let k7memopToSrcrops = function
  | K7_MConst _     -> []
  | K7_MVar _       -> []
  | K7_MFunArg _    -> [K7ROp_IntReg k7rintreg_stackpointer]
  | K7_MStackCell _ -> [K7ROp_IntReg k7rintreg_stackpointer]

let rinstrToSrcrops' = function
  | K7R_IntLoadMem(s,_)		  -> k7memopToSrcrops s @ [K7ROp_MemOp s]
  | K7R_IntStoreMem(s,d)	  -> k7memopToSrcrops d @ [K7ROp_IntReg s] 
  | K7R_IntLoadEA(s,_)		  -> raddrToRops s
  | K7R_IntUnaryOp(op,sd)	  -> intuopToSrcrops (K7ROp_IntReg sd) op
  | K7R_IntUnaryOpMem(op,sd)	  -> k7memopToSrcrops sd @ 
				       intuopToSrcrops (K7ROp_MemOp sd) op
  | K7R_IntCpyUnaryOp(op,s,_)	  -> (K7ROp_IntReg s)::(intcpyuopToSrcrops op)
  | K7R_IntBinOp(_,s,sd)	  -> [K7ROp_IntReg s; K7ROp_IntReg sd]
  | K7R_IntBinOpMem(_,s,sd)	  -> k7memopToSrcrops s @ 
				       [K7ROp_MemOp s; K7ROp_IntReg sd]
  | K7R_CondBranch(cond,_)	  -> branchconditionToSrcrops cond
  | K7R_SimdLoad(_,s,_) 	  -> K7ROp_MemoryState::(raddrToRops s)
  | K7R_SimdStore(s,_,d) 	  -> K7ROp_MemoryState::
					(K7ROp_MMXReg s)::(raddrToRops d)
  | K7R_SimdSpill(s,_) 		  -> [K7ROp_MMXReg s; 
				      K7ROp_IntReg k7rintreg_stackpointer]

  | K7R_SimdCpyUnaryOpMem(op,s,_) -> k7memopToSrcrops s @ 
				       ((K7ROp_MemOp s)::(simdcpyuopToSrcrops op))
  | K7R_SimdUnaryOp(op,sd) 	  -> simduopToSrcrops (K7ROp_MMXReg sd) op
  | K7R_SimdCpyUnaryOp(op,s,_)    -> (K7ROp_MMXReg s)::(simdcpyuopToSrcrops op)

  | K7R_SimdBinOpMem(_,s,sd)	  -> k7memopToSrcrops s @ 
				       [K7ROp_MemOp s; K7ROp_MMXReg sd]
  | K7R_SimdBinOp(_,s,sd) 	  -> [K7ROp_MMXReg s; K7ROp_MMXReg sd]
  | K7R_Label _			  -> []
  | K7R_Jump _			  -> []
  | K7R_FEMMS			  -> []
  | K7R_Ret			  -> []
  | K7R_SimdLoadStoreBarrier	  -> [K7ROp_MemoryState]
  | K7R_SimdPromiseCellSize _     -> []

(* WRITING OF K7ROPS ********************************************************)

let intuopToDstrops d = function
  | K7_IPush  	    -> [(K7ROp_IntReg k7rintreg_stackpointer,1)]
  | K7_IPop  	    -> [(K7ROp_IntReg k7rintreg_stackpointer,1); (d,1)]
  | K7_IClear 	    -> [(d,1); (zeroflag, 1)]
  | K7_IInc 	    -> [(d,1); (zeroflag, 1)]
  | K7_IDec 	    -> [(d,1); (zeroflag, 1)]
  | K7_IAddImm _    -> [(d,1); (zeroflag, 1)]
  | K7_ISubImm _    -> [(d,1); (zeroflag, 1)]
  | K7_IShlImm _    -> [(d,1); (zeroflag, 1)]
  | K7_IShrImm _    -> [(d,1); (zeroflag, 1)]
  | K7_INegate 	    -> [(d,1); (zeroflag, 1)]
  | K7_ILoadValue _ -> [(d,1)]

let intcpyuopToDstrops d = function
  | K7_ICopy	 -> [(d,1)]
  | K7_IMulImm _ -> [(d,5)]

let simduopToLatency = function
  | K7_FPChs _ 	    -> 4
  | K7_FPMulConst _ -> 6

let simdbopToLatency = function
  | K7_FPAdd   -> 4
  | K7_FPSub   -> 4
  | K7_FPSubR  -> 4
  | K7_FPMul   -> 4
  | K7_FPPPAcc -> 4
  | K7_FPNNAcc -> 4
  | K7_FPNPAcc -> 4
  | K7_UnpckLo -> 2
  | K7_UnpckHi -> 2


(* EXPORTED FUNCTIONS *******************************************************)

let k7rinstrToSrck7rops instr =
  let reads = rinstrToSrcrops' instr in
    K7ROp_IP::(if k7rinstrIsMMX instr then K7ROp_MMXState::reads else reads)

let k7rinstrToDstk7rops = function
  | K7R_FEMMS			  -> [(K7ROp_MMXState, 2)]
  | K7R_Ret			  -> [(K7ROp_IP, 4)]
  | K7R_Label _			  -> [(K7ROp_IP, 2); (K7ROp_MemoryState, 0)]
  | K7R_Jump d			  -> [(K7ROp_IP, 4)]
  | K7R_IntLoadMem(_,d)		  -> [(K7ROp_IntReg d, 4)]
  | K7R_IntStoreMem(_,d)	  -> [(K7ROp_MemOp d, 4)]
  | K7R_IntLoadEA(_,d)		  -> [(K7ROp_IntReg d, 2)]
  | K7R_IntUnaryOp(op,sd)	  -> intuopToDstrops (K7ROp_IntReg sd) op
  | K7R_IntUnaryOpMem(op,sd)	  -> intuopToDstrops (K7ROp_MemOp sd) op
  | K7R_IntCpyUnaryOp(op,_,d)	  -> intcpyuopToDstrops (K7ROp_IntReg d) op
  | K7R_IntBinOp(_,_,sd)	  -> [(K7ROp_IntReg sd, 1); (zeroflag, 1)]
  | K7R_IntBinOpMem(_,_,sd)	  -> [(K7ROp_IntReg sd, 4); (zeroflag, 1)]
  | K7R_CondBranch(_,d)		  -> [(K7ROp_IP, 4)]
  | K7R_SimdLoad(_,_,d)	  	  -> [(K7ROp_MMXReg d, 4)]
  | K7R_SimdStore _		  -> []
  | K7R_SimdSpill(_,d)		  -> [(mmxstackcell d, 4)]
  | K7R_SimdUnaryOp(op,sd) 	  -> [(K7ROp_MMXReg sd, simduopToLatency op)]
  | K7R_SimdCpyUnaryOp(op,_,d)    -> [(K7ROp_MMXReg d, 2)]
  | K7R_SimdCpyUnaryOpMem(op,_,d) -> [(K7ROp_MMXReg d, 4)]
  | K7R_SimdBinOp(op,_,sd)	  -> [(K7ROp_MMXReg sd, simdbopToLatency op)]
  | K7R_SimdBinOpMem(op,_,sd)	  -> [(K7ROp_MMXReg sd, simdbopToLatency op+2)]
  | K7R_SimdLoadStoreBarrier	  -> [(K7ROp_MemoryState, 0)]
  | K7R_SimdPromiseCellSize _     -> [(K7ROp_IP, 0)]

let k7rinstrToMaxlatency instr = max_list (map snd (k7rinstrToDstk7rops instr))


(* returns true, if instruction x cannot roll over instruction y. *)
let k7rinstrCannotRollOverK7rinstr x y =
  let (xR, xW) = (k7rinstrToSrck7rops x, map fst (k7rinstrToDstk7rops x))
  and (yR, yW) = (k7rinstrToSrck7rops y, map fst (k7rinstrToDstk7rops y)) in
    (* RAW *) lists_overlap xW yR ||
    (* WAW *) lists_overlap xW yW ||
    (* WAR *) lists_overlap xR yW

