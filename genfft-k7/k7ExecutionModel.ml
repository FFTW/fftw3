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

(* The functions in this module model some aspects of the execution behaviour
 * of the K7 processor. The instruction scheduler uses these functions for
 * selecting instructions that do not require the same execution resources.  *)

open K7Basics
open NonDetMonad

(* Do not care about address generation. (Basically assume that enough 
 * address-generation units (AGUs) are available at any time.) *)

type k7executionresource = 		(* K7 EXECUTION RESOURCE ************)
  | K7E_IEU				(*   Directpath integer instruction *)
  | K7E_FADD				(*   FP-Add, some MMX-instrs	    *)
  | K7E_FMUL				(*   FP-Mul, some MMX-instrs	    *)
  | K7E_FSTORE				(*   FP-Store			    *)
  | K7E_LD				(*   Load Data (dword or qword)     *)
  | K7E_LDST				(*   Load Or Store Data (d/q-word)  *)

(* Per cycle the following resources are available: *)
let k7ResourcesPerCycle =
  [K7E_IEU;  K7E_IEU;  K7E_IEU; 	(* 3 int instrs (directpath decode) *)
   K7E_FADD; K7E_FMUL; K7E_FSTORE; 	(* 2 fp  instrs and one fp-store    *)
   K7E_LD;   K7E_LDST]			(* 2 loads or 1 load and 1 store    *)

(* resources that are available at a given time *) 
let fetchResourcesM = fetchStateM
let storeResourcesM = storeStateM

let consumeNoneM = unitM ()
let consumeOneM r = fetchResourcesM >>= deleteFirstM ((=)r) >>= storeResourcesM
let consumeOneOfM rs = memberM rs >>= consumeOneM
let consumeManyM = iterM consumeOneM 
let consumeAllM = storeResourcesM []

(***************************************************************************)

let loadConsumesM  = consumeOneOfM [K7E_LD; K7E_LDST]
let fpanyConsumesM = consumeOneOfM [K7E_FSTORE; K7E_FMUL; K7E_FADD]
let fpmmxConsumesM = consumeOneOfM [K7E_FMUL; K7E_FADD]

let k7simdunaryopConsumesM = function
  | K7_FPChs _      -> fpmmxConsumesM >> loadConsumesM
  | K7_FPMulConst _ -> consumeOneM K7E_FMUL >> loadConsumesM

let k7simdbinopConsumesM = function
  | K7_UnpckLo | K7_UnpckHi 		 -> fpmmxConsumesM
  | K7_FPAdd   | K7_FPSub   | K7_FPSubR  -> consumeOneM K7E_FADD
  | K7_FPPPAcc | K7_FPNNAcc | K7_FPNPAcc -> consumeOneM K7E_FADD
  | K7_FPMul 				 -> consumeOneM K7E_FMUL

let k7simdcpyunaryopConsumesM = function
  | K7_FPId   -> fpanyConsumesM
  | K7_FPSwap -> fpmmxConsumesM

let k7intcpyunaryopConsumesM = function
  | K7_ICopy     -> consumeOneM K7E_IEU
  | K7_IMulImm _ -> consumeAllM

let k7rinstrConsumesM = function
  | K7R_SimdLoadStoreBarrier	  -> consumeNoneM
  | K7R_SimdPromiseCellSize _ 	  -> consumeNoneM
  | K7R_Ret 			  -> consumeNoneM
  | K7R_FEMMS 			  -> fpanyConsumesM
  | K7R_CondBranch _ 		  -> consumeOneM K7E_IEU
  | K7R_Jump _ 			  -> consumeOneM K7E_IEU
  | K7R_Label _ 		  -> consumeOneM K7E_IEU
  | K7R_IntLoadEA _ 	          -> consumeOneM K7E_IEU
  | K7R_IntBinOp _ 	          -> consumeOneM K7E_IEU
  | K7R_IntUnaryOp _ 	          -> consumeOneM K7E_IEU
  | K7R_IntCpyUnaryOp(op,_,_)     -> k7intcpyunaryopConsumesM op
  | K7R_IntBinOpMem _ 	          -> consumeOneM K7E_IEU >> loadConsumesM
  | K7R_IntUnaryOpMem _           -> consumeOneM K7E_IEU >> loadConsumesM
  | K7R_IntLoadMem _ 	          -> consumeOneM K7E_IEU >> loadConsumesM
  | K7R_IntStoreMem _ 	          -> consumeManyM [K7E_IEU; K7E_LDST]
  | K7R_SimdLoad _ 	          -> loadConsumesM >> fpanyConsumesM
  | K7R_SimdStore _ 	          -> consumeManyM [K7E_FSTORE; K7E_LDST]
  | K7R_SimdSpill _ 	          -> consumeManyM [K7E_FSTORE; K7E_LDST]
  | K7R_SimdBinOp(op,_,_) 	  -> k7simdbinopConsumesM op
  | K7R_SimdBinOpMem(op,_,_)  	  -> k7simdbinopConsumesM op >> loadConsumesM
  | K7R_SimdUnaryOp(op,_)     	  -> k7simdunaryopConsumesM op
  | K7R_SimdCpyUnaryOp(op,_,_)    -> k7simdcpyunaryopConsumesM op
  | K7R_SimdCpyUnaryOpMem(op,_,_) -> k7simdcpyunaryopConsumesM op >> 
				       loadConsumesM

(* EXPORTED FUNCTIONS *******************************************************)

let k7SlotsPerCycle = 3		      (* number of decoding slots per cycle *)

(* checks if a some given instructions can be issued in the same cycle. *)
let k7rinstrsCanIssueInOneCycle instrs =
  List.length instrs <= k7SlotsPerCycle &&
  runP (iterM k7rinstrConsumesM) instrs k7ResourcesPerCycle
