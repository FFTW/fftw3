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

(* This module provides two levels of machine-dependent instruction types 
 * that utilize x86-style address formats. Also,
 *	. 2-operand instruction forms are used.
 *	. unary operations are classified as either 'destructive' or 'copying'.
 *	. almost all binary operations are assumed to be destructive.
 *	  (which is true for almost all int/SIMD binary operations).
 *	. there is one 'copying' integer binary operation ('lea').
 *)

open List
open Util
open Number
open Variable
open VSimdBasics

(* DATA TYPE DEFINITIONS ***************************************************)

type k7stack =				(* STACKS SUPPORTED *****************)
  | K7_INTStack				(*   Integer Stack for Saving Args  *)
  | K7_MMXStack				(*   MMX Stack 			    *)

type k7memop =				(* MEMORY OPERAND *******************)
  | K7_MConst of string			(*   Named Constant (read-only)     *)
  | K7_MVar of string			(*   Named Variable (read-write)    *)
  | K7_MFunArg of			(*   Function Argument (1-based)    *)
	int				(*	= i (the i-th argument)	    *)
  | K7_MStackCell of 			(*   Stackcell (0-based)	    *)
	k7stack * 			(*	= stack type (int/mmx)	    *)
	int				(*	= position on stack	    *)

type k7intunaryop =			(* K7 INTEGER UNARY OPERATION *******)
  | K7_IPush				(*   Store on the Stack		    *)
  | K7_IPop				(*   Load from the Stack	    *)
  | K7_INegate				(*   Negate Signed Integer          *)
  | K7_IClear				(*   Clear Integer Register	    *)
  | K7_IInc				(*   Increment Integer (= add one)  *)
  | K7_IDec				(*   Decrement Integer (= sub one)  *)
  | K7_IAddImm of int			(*   Add Constant to Register	    *)
  | K7_ISubImm of int			(*   Subtract Constant from Reg     *)
  | K7_IShlImm of int			(*   Signed Shift Left by N bits    *)
  | K7_IShrImm of int			(*   Signed Shift Right by N bits   *)
  | K7_ILoadValue of int		(*   Load Const into Int Register   *)

type k7intcpyunaryop =			(* K7 INTEGER COPYING UNARY OP ******)
  | K7_ICopy				(*   Copy Integer Register          *)
  | K7_IMulImm of int			(*   Signed Multiply w/Int-Constant *)

type k7intbinop = 			(* K7 INTEGER BINARY OPERATION ******)
  | K7_IAdd				(*   Signed Integer Addition	    *)
  | K7_ISub				(*   Signed Integer Subtraction     *)

type k7simdunaryop = 			(* K7 FP-SIMD UNARY OPERATION *******)
  | K7_FPChs of vsimdpos		(*   Change Sign 		    *)
  | K7_FPMulConst of			(*   Multiply with 2-way Constant:  *)
	number *			(*	= lower part of constant    *)
	number				(*	= upper part of constant    *)

type k7simdcpyunaryop =			(* K7 FP-SIMD COPYING UNARY OP ******)
  | K7_FPId				(*   Copy Simd Register		    *)
  | K7_FPSwap				(*   Copy Reg and Swap Doublewords  *)

type k7simdbinop = 			(* K7 SIMD BINARY OPERATION *********)
  | K7_FPAdd 				(*   FP Addition		    *)
  | K7_FPSub 				(*   FP Subtraction		    *)
  | K7_FPSubR 				(*   FP "Reversed" Subtraction	    *)
  | K7_FPMul 				(*   FP Multiplication		    *)
  | K7_FPPPAcc 				(*   FP Pos-Pos Accumulation        *)
  | K7_FPNNAcc 				(*   FP Neg-Neg Accumulation	    *)
  | K7_FPNPAcc 				(*   FP Neg-Pos Accumulation	    *)
  | K7_UnpckLo				(*   Join Lo Parts of 2 Simd Words  *)
  | K7_UnpckHi				(*   Join Hi Parts of 2 Simd Words  *)

type k7operandsize =				(* K7 OPERAND SIZE **********)
  | K7_DWord					(*   Double Word (32 bit)   *)
  | K7_QWord					(*   Quad Word (64 bit)	    *)

type k7branchcondition =			(* K7 BRANCH CONDITION ******)
  | K7_BCond_NotZero
  | K7_BCond_Zero
  | K7_BCond_GreaterZero
  | K7_BCond_EqualZero


(****************************************************************************
 * K7V DATA TYPES ***********************************************************
 ****************************************************************************)

type k7vaddr =					(* K7 VIRTUAL ADDRESS *******)
  | K7V_RID of vintreg * int 			(*   %2(%1)		    *)
  | K7V_SID of vintreg * int * int		(*   %3(,%1,%2)		    *)
  | K7V_RISID of vintreg * vintreg * int * int 	(*   %4(%1,%2,%3)	    *)

type k7vbranchtarget =			(* K7 VIRTUAL BRANCH TARGET *********)
  | K7V_BTarget_Named of string		(*   Named Branch Target (string)   *)

type k7vinstr = 			(* K7 VIRTUAL INSTRUCTION ***********)
  | K7V_IntLoadMem of 			(*   Load Integer from Memory	    *)
	k7memop *			(*	= source operand (s_mem)    *)
	vintreg				(*	= dest register (d_reg)     *)
  | K7V_IntStoreMem of 			(*   Store Integer to Memory	    *)
	vintreg *			(*	= source register (s)	    *)
	k7memop				(*	= destination (d_mem)	    *)
  | K7V_IntLoadEA of			(*   Load Effective Address	    *)
	k7vaddr *			(*      = source address (s_mem)    *)
	vintreg				(*	= destination register (d)  *)
  | K7V_IntUnaryOp of 			(*   Destructive Int Unary Op       *)
	k7intunaryop *			(*      = operation identifier (op) *) 
	vintreg				(*	= source/dest register (sd) *)
  | K7V_IntUnaryOpMem of 		(*   Destructive Int Unary Op w/Mem *)
	k7intunaryop *			(*      = operation identifier (op) *)
	k7memop				(*	= src/dest operand (sd_mem) *)
  | K7V_IntCpyUnaryOp of 		(*   Copying Int Unary Operation    *)
	k7intcpyunaryop *		(*      = operation identifier (op) *)
	vintreg *			(*      = source register (s)	    *)
	vintreg				(*	= destination register (d)  *)
  | K7V_IntBinOp of			(*   Integer Binary Operation	    *)
	k7intbinop *			(* 	= operation identifier (op) *)
	vintreg *			(* 	= source register (s)	    *)
	vintreg				(*	= source/dest register (sd) *)
  | K7V_IntBinOpMem of			(*   Integer Binary Operation w/Mem *)
	k7intbinop *			(* 	= operation identifier (op) *)
	k7memop *			(* 	= source operand (s_mem)    *)
	vintreg				(*	= source/dest register (sd) *)
  | K7V_SimdLoad of 			(*   SIMD Load Data		    *)
	k7operandsize *			(*	= operand size (s_size)	    *)
	k7vaddr * 			(*	= source address (s_addr)   *)
	vsimdreg			(*	= destination register (d)  *)
  | K7V_SimdStore of			(*   SIMD Store Data		    *)
	vsimdreg * 			(*	= source register (s)	    *)
	k7operandsize *			(*	= operand size (d_size)	    *)
	k7vaddr				(*	= dest address (d_addr)     *)
  | K7V_SimdUnaryOp of 			(*   Destructive SIMD Unary Op	    *)
	k7simdunaryop *			(*	= operation identifier (op) *)
	vsimdreg 			(*	= source/dest register (sd) *)
  | K7V_SimdCpyUnaryOp of 		(*   Copying SIMD Unary Operation   *)
	k7simdcpyunaryop *		(*	= operation identifier (op) *)
	vsimdreg *			(*	= source register (s)	    *)
	vsimdreg 			(*	= destination register (d)  *)
  | K7V_SimdCpyUnaryOpMem of		(*   Copying SIMD Unary Op w/Mem    *)
	k7simdcpyunaryop *		(*	= operation identifier (op) *)
	k7memop *			(*	= source operand (s_memop)  *)
	vsimdreg 			(*	= destination register (d)  *)
  | K7V_SimdBinOp of			(*   SIMD Binary Operation	    *)
	k7simdbinop *			(*	= operation identifier (op) *)
	vsimdreg *			(*      = source register (s)	    *)
	vsimdreg			(*	= source/dest register (sd) *)
  | K7V_SimdBinOpMem of			(*   SIMD Binary Operation w/Mem    *)
	k7simdbinop *			(*	= operation identifier (op) *)
	k7memop *			(*      = source operand (s_memop)  *)
	vsimdreg			(*	= source/dest register (sd) *)
  | K7V_SimdLoadStoreBarrier		(*   SIMD Load/Store Barrier	    *)
  | K7V_RefInts of			(*   Reference Integer Registers    *)
	vintreg list			(*	= source registers (s) 	    *)
  | K7V_Label of			(*   Label (Named Branch Target)    *)
	string				(*	= label name (s_str)	    *)
  | K7V_Jump of				(*   Unconditional Branch	    *)
	k7vbranchtarget			(*	= branch target 	    *)
  | K7V_CondBranch of			(*   Conditional Branch		    *)
	k7branchcondition *		(*	= branch condition	    *)
	k7vbranchtarget			(*	= branch target		    *)
  | K7V_SimdPromiseCellSize of 		(*   Specify Size of MMX Stackcells *)
	k7operandsize


(****************************************************************************
 * K7R DATA TYPES ***********************************************************
 ****************************************************************************)

type k7rmmxreg = K7R_MMXReg of string	     (* K7 REAL MMX (SIMD) REGISTER *)
type k7rintreg = K7R_IntReg of string	     (* K7 REAL INTEGER REGISTER    *)

let toK7rintreg i = K7R_IntReg i
let toK7rmmxreg m = K7R_MMXReg m

module RIntRegMap  = Map.Make(struct type t=k7rintreg let compare=compare end)
module RSimdRegMap = Map.Make(struct type t=k7rmmxreg let compare=compare end)

(* The modes 'RID' and 'SID' exist because they introduce fewer 
 * dependencies between registers. *)
type k7raddr =						(* K7 REAL ADDRESS **)
  | K7R_RID of k7rintreg * int 				(*   %2(%1)	    *)
  | K7R_SID of k7rintreg * int * int			(*   %3(,%1,%2)	    *)
  | K7R_RISID of k7rintreg * k7rintreg * int * int	(*   %4(%1,%2,%3)   *)

type k7rbranchtarget =			(* K7 REAL BRANCH TARGET ************)
  | K7R_BTarget_Named of string		(*   Named Branch Target (string)   *)

type k7rinstr =				(* K7 REAL INSTRUCTION **************)
  | K7R_IntLoadMem of			(*   Load Integer From Memory	    *)
	k7memop *			(*	= source operand (s_memop)  *)
	k7rintreg			(*	= destination register (d)  *)
  | K7R_IntStoreMem of			(*   Store Integer To Memory	    *)
	k7rintreg *			(*	= source register (s)	    *)
	k7memop				(*	= destination (d_memop)	    *)
  | K7R_IntLoadEA of 			(*   Load Effective Address	    *)
	k7raddr *			(*	= source address (s_addr)   *)
	k7rintreg			(*	= destination register (d)  *)
  | K7R_IntUnaryOp of			(*   Destructive Integer Unary Op   *)
	k7intunaryop *			(*	= operation identifier (op) *)
	k7rintreg			(*	= source/dest register (sd) *)
  | K7R_IntUnaryOpMem of		(*   Destructive Int Unary Op w/Mem *)
	k7intunaryop *			(*	= operation identifier (op) *)
	k7memop				(*	= source/dest (sd_memop)    *)
  | K7R_IntCpyUnaryOp of		(*   Copying Integer Unary Op	    *)
	k7intcpyunaryop *		(*	= operation identifier (op) *)
	k7rintreg *			(*	= source register (s)	    *)
	k7rintreg			(*	= destination register (d)  *)
  | K7R_IntBinOp of			(*   Integer Binary Operation	    *)
	k7intbinop *			(*	= operation identifier (op) *)
	k7rintreg *			(*	= source register (s)	    *)
	k7rintreg 			(*	= source/dest register (sd) *)
  | K7R_IntBinOpMem of			(*   Int Binary Op w/Mem Operand    *)
	k7intbinop *			(*	= operation identifier (op) *)
	k7memop *			(*	= source operand (s_memop)  *)
	k7rintreg 			(*	= source/dest register (sd) *)
  | K7R_SimdLoad of 			(*   SIMD Load Data		    *)
	k7operandsize *			(*	= operand size (s_size)	    *)
	k7raddr * 			(*	= source address (s_addr)   *)
	k7rmmxreg			(*	= dest register (d_reg)	    *)
  | K7R_SimdStore of 			(*   SIMD Store Data		    *)
	k7rmmxreg * 			(*	= source register (s)	    *)
	k7operandsize *			(*	= operand size (d_size)	    *)
	k7raddr				(*	= destination addr (d_addr) *)
  | K7R_SimdSpill of 			(*   Spill SIMD Register To Stack   *)
	k7rmmxreg * 			(*	= source register (s)	    *)
	int				(*	= dest cell-index (d_idx)   *)
  | K7R_SimdUnaryOp of			(*   SIMD Destructive Unary Op	    *)
	k7simdunaryop *			(*	= operation identifier (op) *)
	k7rmmxreg			(*	= source/dest register (sd) *)
  | K7R_SimdCpyUnaryOp of		(*   SIMD Copying Unary Op	    *)
	k7simdcpyunaryop *		(*	= operation identifier (op) *)
	k7rmmxreg *			(*	= source register (s)	    *)
	k7rmmxreg			(*	= destination register (d)  *)
  | K7R_SimdCpyUnaryOpMem of		(*   SIMD Copying Unary Op w/Mem    *)
	k7simdcpyunaryop *		(*	= operation identifier (op) *)
	k7memop *			(*	= source operand (s_memop)  *)
	k7rmmxreg			(*	= destination register (d)  *)
  | K7R_SimdBinOp of 			(*   SIMD Binary Operation	    *)
	k7simdbinop * 			(*	= operation identifier (op) *)
	k7rmmxreg * 			(*	= source register (s)	    *)
	k7rmmxreg			(*	= source/dest register (sd) *)
  | K7R_SimdBinOpMem of 		(*   SIMD Binary Op w/Mem Operand   *)
	k7simdbinop *			(*	= operation identifier (op) *)
	k7memop *			(*	= source operand (s_memop)  *)
	k7rmmxreg			(*	= source/dest register (d)  *)
  | K7R_SimdLoadStoreBarrier		(*   SIMD Load/Store Barrier	    *)
  | K7R_Label of			(*   Label (Named Branch Target)    *)
	string				(*	= label name (s_str)	    *)
  | K7R_Jump of				(*   Unconditional Branch	    *)
	k7rbranchtarget			(*	= branch target 	    *)
  | K7R_CondBranch of			(*   Conditional Branch		    *)
	k7branchcondition *		(*	= branch condition	    *)
	k7rbranchtarget			(*	= branch target		    *)
  | K7R_FEMMS				(*   Switch From/To MMX-Mode	    *)
  | K7R_Ret				(*   Return From Subroutine	    *)
  | K7R_SimdPromiseCellSize of 		(*   Specify Size of MMX Stackcells *)
	k7operandsize


(* DEFINITION OF CONSTANTS **************************************************)

let k7rmmxregs_names = ["mm0"; "mm1"; "mm2"; "mm3"; "mm4"; "mm5"; "mm6"; "mm7"]
let k7rintregs_names = ["eax"; "ecx"; "edx"; "ebx"; "esi"; "edi"; "ebp"]
let k7rintregs_calleesaved_names = ["ebx"; "esi"; "edi"; "ebp"]
let k7rintreg_stackpointer_name  = "esp"

let k7rmmxregs = map toK7rmmxreg k7rmmxregs_names
let k7rintregs = map toK7rintreg k7rintregs_names
let k7rintregs_calleesaved = map toK7rintreg k7rintregs_calleesaved_names
let k7rintreg_stackpointer = toK7rintreg k7rintreg_stackpointer_name
let retval = toK7rintreg "eax"

(* FUNCTIONS OPERATING ON VALUES OF TYPE K7... ******************************)

let k7operandsizeToString = function
  | K7_DWord -> "K7_DWord"
  | K7_QWord -> "K7_QWord"

let k7operandsizeToInteger = function
  | K7_DWord -> 4
  | K7_QWord -> 8

let k7simdbinopIsParallel = function
  | K7_FPAdd   -> true
  | K7_FPSub   -> true
  | K7_FPSubR  -> true
  | K7_FPMul   -> true
  | K7_FPPPAcc -> false
  | K7_FPNPAcc -> false
  | K7_FPNNAcc -> false
  | K7_UnpckLo -> false
  | K7_UnpckHi -> false

let k7simdbinopToParallel = function
  | K7_FPAdd   -> K7_FPAdd
  | K7_FPSub   -> K7_FPSubR
  | K7_FPSubR  -> K7_FPSub
  | K7_FPMul   -> K7_FPMul
  | K7_FPPPAcc -> failwith "k7simdbinopToParallel: ppacc is not parallel!"
  | K7_FPNPAcc -> failwith "k7simdbinopToParallel: npacc is not parallel!"
  | K7_FPNNAcc -> failwith "k7simdbinopToParallel: nnacc is not parallel!"
  | K7_UnpckLo -> failwith "k7simdbinopToParallel: unpcklo is not parallel!"
  | K7_UnpckHi -> failwith "k7simdbinopToParallel: unpckhi is not parallel!"

let k7simdbinopFlops = function
  | K7_FPAdd   
  | K7_FPSub   
  | K7_FPSubR 
  | K7_FPPPAcc
  | K7_FPNPAcc
  | K7_FPNNAcc -> (1, 0)
  | K7_FPMul   -> (0, 1)
  | _          -> (0, 0)

let k7vFlops l =
  let one = function
    | K7V_SimdBinOp (x, _, _) -> k7simdbinopFlops x
    | K7V_SimdBinOpMem (x, _, _) -> k7simdbinopFlops x
    | K7V_SimdUnaryOp ((K7_FPMulConst _), _) -> (0, 1)
    | _ -> (0, 0)
  in
  List.fold_left (fun (a, m) (a1, m1) -> (a + a1, m + m1)) (0, 0) 
    (List.map one l)

(* FUNCTIONS OPERATING ON VALUES OF TYPE K7V... *****************************)

let vsimdunaryopToK7simdcpyunaryop = function
  | V_Id	 -> K7_FPId
  | V_Swap	 -> K7_FPSwap
  | V_Chs _	 -> failwith "vsimdunaryopToK7simdcpyunaryop (1)"
  | V_MulConst _ -> failwith "vsimdunaryopToK7simdcpyunaryop (2)"

let vsimdbinopToK7simdbinop = function
  | V_Add     -> K7_FPAdd
  | V_Sub     -> K7_FPSub
  | V_SubR    -> K7_FPSubR
  | V_Mul     -> K7_FPMul
  | V_PPAcc   -> K7_FPPPAcc
  | V_NNAcc   -> K7_FPNNAcc
  | V_NPAcc   -> K7_FPNPAcc
  | V_UnpckLo -> K7_UnpckLo
  | V_UnpckHi -> K7_UnpckHi

let k7vaddrToK7raddr map = function
  | K7V_RID(base,ofs)		   -> K7R_RID(map base, ofs)
  | K7V_SID(idx,scalar,ofs)	   -> K7R_SID(map idx, scalar, ofs)
  | K7V_RISID(base,idx,scalar,ofs) -> K7R_RISID(map base, map idx, scalar, ofs)

let k7vaddrToSimplified = function
  | K7V_SID(index,2,ofs)	   -> K7V_RISID(index,index,1,ofs)
  | K7V_RISID(base,index,0,ofs)	   -> K7V_RID(base,ofs)
  | vaddr 			   -> vaddr

let k7vaddrToVintregs = function	(* output may contain duplicates *)
  | K7V_RID(base,_)	      -> [base]
  | K7V_SID(base,_,_)	      -> [base]
  | K7V_RISID(base,index,_,_) -> [base; index]

let k7vbranchtargetToVintregs (K7V_BTarget_Named _) = []

let k7vinstrToSrcvregs = function	(* output may contain duplicates *)
  | K7V_SimdLoadStoreBarrier  -> [],	 []
  | K7V_SimdPromiseCellSize _ -> [],	 []
  | K7V_Label _		      -> [],     []
  | K7V_RefInts xs	      -> [],     xs
  | K7V_CondBranch(cond,d)    -> [],     k7vbranchtargetToVintregs d
  | K7V_Jump d		      -> [],     k7vbranchtargetToVintregs d
  | K7V_IntCpyUnaryOp(_,s,_)  -> [],	 [s]
  | K7V_IntUnaryOp(K7_IPop,_) -> [],	 []
  | K7V_IntUnaryOp(_,sd)      -> [],	 [sd]
  | K7V_IntUnaryOpMem _	      -> [],	 []
  | K7V_IntBinOp(_,s,sd)      -> [],	 [s;sd]
  | K7V_IntBinOpMem(_,_,sd)   -> [],     [sd]
  | K7V_IntLoadEA(s,_) 	      -> [],	 k7vaddrToVintregs s
  | K7V_IntLoadMem _	      -> [],	 []
  | K7V_IntStoreMem(s,_)      -> [],	 [s]
  | K7V_SimdLoad(_,s,_)       -> [],	 k7vaddrToVintregs s
  | K7V_SimdStore(s,_,d)      -> [s],    k7vaddrToVintregs d
  | K7V_SimdBinOp(_,s,sd)     -> [s;sd], []
  | K7V_SimdBinOpMem(_,_,sd)  -> [sd],   []
  | K7V_SimdUnaryOp(_,sd)     -> [sd],   []
  | K7V_SimdCpyUnaryOp(_,s,_) -> [s],    []
  | K7V_SimdCpyUnaryOpMem _   -> [],     []

let k7vinstrToDstvregs = function	(* output may contain duplicates :-) *)
  | K7V_SimdLoadStoreBarrier	 -> [],	  []
  | K7V_SimdPromiseCellSize _    -> [],   []
  | K7V_Label _		         -> [],   []
  | K7V_RefInts _	         -> [],   []
  | K7V_CondBranch(cond,d)       -> [],   []
  | K7V_Jump d		         -> [],   []
  | K7V_IntLoadMem(_,d)		 -> [],   [d]
  | K7V_IntStoreMem _		 -> [],   []
  | K7V_IntLoadEA(_,d)		 -> [],   [d]
  | K7V_IntUnaryOp(K7_IPush,_) 	 -> [],   []
  | K7V_IntUnaryOp(_,sd) 	 -> [],   [sd]
  | K7V_IntUnaryOpMem _		 -> [],   []
  | K7V_IntCpyUnaryOp(_,_,d)     -> [],   [d]
  | K7V_IntBinOp(_,_,sd) 	 -> [],   [sd]
  | K7V_IntBinOpMem _		 -> [],	  []
  | K7V_SimdLoad(_,_,d) 	 -> [d],  []
  | K7V_SimdStore _  		 -> [],   []
  | K7V_SimdBinOp(_,_,sd) 	 -> [sd], []
  | K7V_SimdBinOpMem(_,_,sd) 	 -> [sd], []
  | K7V_SimdUnaryOp(_,sd) 	 -> [sd], []
  | K7V_SimdCpyUnaryOp(_,_,d)    -> [d],  []
  | K7V_SimdCpyUnaryOpMem(_,_,d) -> [d],  []

(* the lists returned do not have duplicates. *)
let k7vinstrToVregs instr = 
  let (s_simd0,s_int0) = k7vinstrToSrcvregs instr in
  let (s_simd, s_int) as src_regs = uniq s_simd0, uniq s_int0 in
  let (d_simd0,d_int0) = k7vinstrToDstvregs instr in
  let (d_simd,d_int) as dst_regs = diff d_simd0 s_simd, diff d_int0 s_int in
    (src_regs, dst_regs, src_regs @. dst_regs)


(* FUNCTIONS OPERATING ON VALUES OF TYPE K7R... *****************************)

let k7raddrToK7rintregs = function	(* output may contain duplicates *)
  | K7R_RID(base,_)	      -> [base]
  | K7R_SID(base,_,_)	      -> [base]
  | K7R_RISID(base,index,_,_) -> [base; index]

let k7raddrToSimplified = function
  | K7R_SID(index,2,ofs)	-> K7R_RISID(index,index,1,ofs)
  | K7R_RISID(base,index,0,ofs)	-> K7R_RID(base,ofs)
  | raddr 			-> raddr

(* returns pair (simds,ints) *)
let k7rinstrToSrck7rregs = function
  | K7R_SimdLoadStoreBarrier  -> [],	 []
  | K7R_SimdPromiseCellSize _ -> [],     []
  | K7R_IntLoadMem _ 	      -> [], 	 []
  | K7R_IntStoreMem(s,_)      -> [], 	 [s]
  | K7R_IntLoadEA(s,_) 	      -> [], 	 k7raddrToK7rintregs s
  | K7R_IntUnaryOp(_,sd)      -> [], 	 [sd]		(* conservative *)
  | K7R_IntUnaryOpMem _       -> [], 	 []
  | K7R_IntCpyUnaryOp(_,s,_)  -> [], 	 [s]
  | K7R_IntBinOp(_,s,sd)      -> [], 	 [s;sd]
  | K7R_IntBinOpMem(_,_,sd)   -> [], 	 [sd]
  | K7R_SimdLoad(_,s,_)       -> [], 	 k7raddrToK7rintregs s
  | K7R_SimdStore(s,_,d)      -> [s], 	 k7raddrToK7rintregs d
  | K7R_SimdSpill(s,_) 	      -> [s], 	 []
  | K7R_SimdUnaryOp(_,sd)     -> [sd], 	 []
  | K7R_SimdCpyUnaryOp(_,s,_) -> [s], 	 []
  | K7R_SimdCpyUnaryOpMem _   -> [], 	 []
  | K7R_SimdBinOp(_,s,sd)     -> [s;sd], []
  | K7R_SimdBinOpMem(_,_,sd)  -> [sd], 	 []
  | K7R_Label _ 	      -> [], 	 []
  | K7R_Jump _ 		      -> [], 	 []
  | K7R_CondBranch _ 	      -> [], 	 []
  | K7R_FEMMS 		      -> [],     []
  | K7R_Ret 		      -> [],     []

(* returns pair (simds,ints) *)
let k7rinstrToDstk7rregs = function
  | K7R_SimdLoadStoreBarrier     -> [],   []
  | K7R_SimdPromiseCellSize _    -> [],   []
  | K7R_IntLoadMem(_,d)		 -> [],   [d]
  | K7R_IntStoreMem _   	 -> [],   []
  | K7R_IntLoadEA(_,d) 		 -> [],   [d]
  | K7R_IntUnaryOp(_,sd)	 -> [],   [sd]		(* conservative *)
  | K7R_IntUnaryOpMem _ 	 -> [],   []
  | K7R_IntCpyUnaryOp(_,_,d)     -> [],   [d]
  | K7R_IntBinOp(_,_,sd)	 -> [],   [sd]
  | K7R_IntBinOpMem(_,_,sd)      -> [],   [sd]
  | K7R_SimdLoad(_,_,d)		 -> [d],  []
  | K7R_SimdStore _		 -> [],   []
  | K7R_SimdSpill _ 		 -> [],   []
  | K7R_SimdUnaryOp(_,sd)	 -> [sd], []
  | K7R_SimdCpyUnaryOp(_,_,d)	 -> [d],  []
  | K7R_SimdCpyUnaryOpMem(_,_,d) -> [d],  []
  | K7R_SimdBinOp(_,_,sd)	 -> [sd], []
  | K7R_SimdBinOpMem(_,_,sd)	 -> [sd], []
  | K7R_Label _			 -> [],   []
  | K7R_Jump _ 			 -> [],   []
  | K7R_CondBranch _ 		 -> [],   []
  | K7R_FEMMS 			 -> [],   []
  | K7R_Ret 			 -> [],   []

let k7rinstrIsMMX = function
  | K7R_SimdLoadStoreBarrier  -> true
  | K7R_SimdPromiseCellSize _ -> true
  | K7R_FEMMS		      -> true
  | K7R_SimdLoad _	      -> true
  | K7R_SimdStore _	      -> true
  | K7R_SimdSpill _	      -> true
  | K7R_SimdUnaryOp _	      -> true
  | K7R_SimdCpyUnaryOp _      -> true
  | K7R_SimdCpyUnaryOpMem _   -> true
  | K7R_SimdBinOp _	      -> true
  | K7R_SimdBinOpMem _	      -> true
  | K7R_IntLoadMem _	      -> false
  | K7R_IntStoreMem _	      -> false
  | K7R_IntLoadEA _	      -> false
  | K7R_IntUnaryOp _	      -> false
  | K7R_IntUnaryOpMem _       -> false
  | K7R_IntCpyUnaryOp _	      -> false
  | K7R_IntBinOp _	      -> false
  | K7R_IntBinOpMem _	      -> false
  | K7R_Label _		      -> false
  | K7R_Jump _		      -> false
  | K7R_CondBranch _	      -> false
  | K7R_Ret		      -> false

let k7rinstrReadsK7rreg sel rreg instr  = 
  mem rreg (sel (k7rinstrToSrck7rregs instr))

let k7rinstrWritesK7rreg sel rreg instr = 
  mem rreg (sel (k7rinstrToDstk7rregs instr))

let k7rinstrReadsK7rmmxreg rreg instr  = k7rinstrReadsK7rreg fst rreg instr
let k7rinstrReadsK7rintreg rreg instr  = k7rinstrReadsK7rreg snd rreg instr

let k7rinstrWritesK7rmmxreg rreg instr = k7rinstrWritesK7rreg fst rreg instr
let k7rinstrWritesK7rintreg rreg instr = k7rinstrWritesK7rreg snd rreg instr

let k7rinstrUsesK7rmmxreg rreg instr = 
  k7rinstrReadsK7rmmxreg rreg instr || k7rinstrWritesK7rmmxreg rreg instr

let k7rinstrUsesK7rintreg rreg instr = 
  k7rinstrReadsK7rintreg rreg instr || k7rinstrWritesK7rintreg rreg instr

