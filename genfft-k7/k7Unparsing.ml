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

(* This module includes functions for unparsing k7vinstrs and k7rinstrs. *)

open List
open Util
open Variable
open Number
open VSimdBasics
open VSimdUnparsing
open K7Basics
open Printf


(* CODE FOR UNPARSING (K7) *************************************************)

let mmx_reg_size = ref K7_QWord

let stackpointer_adjustment = ref 0

let k7memopToString = function
  | K7_MVar var_name -> 
      var_name
  | K7_MConst const_name -> 
      const_name
  | K7_MStackCell(K7_INTStack,_) -> 
      failwith "k7memopToString: unsupported!"
  | K7_MFunArg i -> 
      sprintf "%d(%%esp)" (i*4 + !stackpointer_adjustment)
  | K7_MStackCell(K7_MMXStack,p) -> 
      sprintf "%d(%%esp)" (k7operandsizeToInteger !mmx_reg_size * p)

let k7operandsizeToMovstring = function 
  | K7_DWord -> "movd"
  | K7_QWord -> "movq"

let k7intbinopToString = function 
  | K7_IAdd -> "addl"  
  | K7_ISub -> "subl"

let k7intunaryopToString = function
  | K7_IPush	    -> "pushl"
  | K7_IPop	    -> "popl"
  | K7_INegate	    -> "negl"
  | K7_IInc    	    -> "incl"
  | K7_IDec    	    -> "decl"
  | K7_IClear  	    -> "clrl"
  | K7_IAddImm _    -> "addl"
  | K7_ISubImm _    -> "subl"
  | K7_IShlImm _    -> "sall"
  | K7_IShrImm _    -> "sral"
  | K7_ILoadValue _ -> "movl"

let k7intcpyunaryopToString = function
  | K7_ICopy     -> "movl"
  | K7_IMulImm _ -> "imull"

let k7simdcpyunaryopToString = function 
  | K7_FPId   -> "movq"
  | K7_FPSwap -> "pswapd"

let k7simdcpyunaryopmemToString = function 
  | K7_FPSwap -> "pswapd"
  | K7_FPId   -> k7operandsizeToMovstring !mmx_reg_size

let k7simdunaryopToString = function
  | K7_FPChs _	    -> "pxor"
  | K7_FPMulConst _ -> "pfmul"

let k7simdbinopToString = function
  | K7_FPAdd   -> "pfadd"
  | K7_FPSub   -> "pfsub"
  | K7_FPSubR  -> "pfsubr"
  | K7_FPMul   -> "pfmul"
  | K7_FPPPAcc -> "pfacc"
  | K7_FPNNAcc -> "pfnacc"
  | K7_FPNPAcc -> "pfpnacc"
  | K7_UnpckLo -> "punpckldq"
  | K7_UnpckHi -> "punpckhdq"

let k7branchconditionToString = function
  | K7_BCond_NotZero     -> "nz"
  | K7_BCond_Zero        -> "z"
  | K7_BCond_GreaterZero -> "g"
  | K7_BCond_EqualZero   -> "e"


(* CODE SHARED FOR UNPARSING K7V AND K7R INSTRUCTIONS ***********************)

let k7mmxstackcellToString c = k7memopToString (K7_MStackCell(K7_MMXStack,c))

let k7simdunaryopToArgstring = function
  | K7_FPChs p		 -> vsimdposToChsconstnamestring p
  | K7_FPMulConst(n1,n2) -> Number.to_konst n1 ^ Number.to_konst n2

let k7intunaryopToArgstrings = function
  | K7_IAddImm c    -> ["$" ^ intToString c]
  | K7_ISubImm c    -> ["$" ^ intToString c]
  | K7_IShlImm c    -> ["$" ^ intToString c]
  | K7_IShrImm c    -> ["$" ^ intToString c]
  | K7_ILoadValue c -> ["$" ^ intToString c]
  | K7_INegate	    -> []
  | K7_IInc	    -> []
  | K7_IDec	    -> []
  | K7_IClear	    -> []
  | K7_IPush	    -> []
  | K7_IPop	    -> []

let k7intcpyunaryopToArgstrings = function
  | K7_ICopy	 -> []
  | K7_IMulImm i -> ["$" ^ intToString i]

(* needed for k7vaddrToString/k7raddrToString *)
let offsetToString x = if x = 0 then "" else intToString x
let scalarToString x = if x <> 1 then "," ^ intToString x else ""


(* CODE FOR UNPARSING (K7V) ************************************************)

let k7vaddrToString vaddr = match k7vaddrToSimplified vaddr with
  | K7V_RID(base,ofs) -> 
      sprintf "%s(%s)" 
	      (offsetToString ofs) 
	      (vintregToString base)
  | K7V_SID(idx,scalar,ofs) -> 
      sprintf "%s(,%s%s)" 
	      (offsetToString ofs) 
	      (vintregToString idx) 
	      (scalarToString scalar)
  | K7V_RISID(base,idx,scalar,ofs) -> 
      sprintf "%s(%s,%s%s)" 
	      (offsetToString ofs) 
	      (vintregToString base) 
	      (vintregToString idx) 
	      (scalarToString scalar)

let k7vbranchtargetToString (K7V_BTarget_Named name) = name

let k7vinstrToArgstrings = function
  | K7V_IntLoadEA(s,d) 		 -> [k7vaddrToString s; vintregToString d]
  | K7V_IntCpyUnaryOp(op,s,d) 	 -> k7intcpyunaryopToArgstrings op @
				    [vintregToString s; vintregToString d]
  | K7V_IntUnaryOp(op,sd) 	 -> k7intunaryopToArgstrings op @
				    [vintregToString sd]
  | K7V_IntUnaryOpMem(op,sd)	 -> k7intunaryopToArgstrings op @
				    [k7memopToString sd]
  | K7V_IntBinOp(_,s,sd)	 -> [vintregToString s; vintregToString sd]
  | K7V_IntBinOpMem(_,s,sd)	 -> [k7memopToString s; vintregToString sd]
  | K7V_IntLoadMem(s,d)		 -> [k7memopToString s; vintregToString d]
  | K7V_IntStoreMem(s,d)	 -> [vintregToString s; k7memopToString d]
  | K7V_SimdLoad(_,s,d)		 -> [k7vaddrToString s; vsimdregToString d]
  | K7V_SimdStore(s,_,d)	 -> [vsimdregToString s; k7vaddrToString d]
  | K7V_SimdUnaryOp(op,sd)	 -> [k7simdunaryopToArgstring op;
				     vsimdregToString sd]
  | K7V_SimdCpyUnaryOp(_,s,d)	 -> [vsimdregToString s; vsimdregToString d]
  | K7V_SimdCpyUnaryOpMem(_,s,d) -> [k7memopToString s; vsimdregToString d]
  | K7V_SimdBinOp(_,s,sd)  	 -> [vsimdregToString s; vsimdregToString sd]
  | K7V_SimdBinOpMem(_,s,sd)	 -> [k7memopToString s; vsimdregToString sd]
  | K7V_RefInts xs		 -> map vintregToString xs
  | K7V_Label _			 -> []
  | K7V_SimdLoadStoreBarrier     -> []
  | K7V_Jump d			 -> [k7vbranchtargetToString d]
  | K7V_CondBranch(_,d)	 	 -> [k7vbranchtargetToString d]
  | K7V_SimdPromiseCellSize size -> [k7operandsizeToString size]

let k7vinstrToInstrstring = function
  | K7V_IntLoadEA _		  -> "lea"
  | K7V_IntUnaryOp(op,_) 	  -> k7intunaryopToString op
  | K7V_IntUnaryOpMem(op,_) 	  -> k7intunaryopToString op
  | K7V_IntCpyUnaryOp(op,_,_) 	  -> k7intcpyunaryopToString op
  | K7V_IntBinOp(op,_,_) 	  -> k7intbinopToString op
  | K7V_IntBinOpMem(op,_,_) 	  -> k7intbinopToString op
  | K7V_IntLoadMem _		  -> "movl"
  | K7V_IntStoreMem _		  -> "movl"
  | K7V_SimdLoad(size,_,_)	  -> k7operandsizeToMovstring size
  | K7V_SimdStore(_,size,_)	  -> k7operandsizeToMovstring size
  | K7V_SimdUnaryOp(op,_) 	  -> k7simdunaryopToString op
  | K7V_SimdCpyUnaryOp(op,_,_)    -> k7simdcpyunaryopToString op
  | K7V_SimdCpyUnaryOpMem(op,_,_) -> k7simdcpyunaryopmemToString op
  | K7V_SimdBinOp(op,_,_) 	  -> k7simdbinopToString op
  | K7V_SimdBinOpMem(op,_,_) 	  -> k7simdbinopToString op
  | K7V_RefInts _		  -> "refints"
  | K7V_Label lbl		  -> lbl ^ ":"
  | K7V_Jump _			  -> "jmp"
  | K7V_CondBranch(cond,_)	  -> "j" ^ k7branchconditionToString cond
  | K7V_SimdLoadStoreBarrier 	  -> "loadstorebarrier"
  | K7V_SimdPromiseCellSize _     -> "promise_mmx_stackcell_size"

let k7vinstrToString i =
  (k7vinstrToInstrstring i) ^ 
	" " ^ (stringlistToString ", " (k7vinstrToArgstrings i))


(* CODE FOR UNPARSING (K7R) *************************************************)

let k7rintregToRawString (K7R_IntReg name) = name
let k7rmmxregToRawString (K7R_MMXReg name) = name

let k7rintregToString (K7R_IntReg name) = "%" ^ name
let k7rmmxregToString (K7R_MMXReg name) = "%" ^ name

(* warning: addresses with base ''%esp'' are *not* adjusted here. *)
let k7raddrToString raddr = match k7raddrToSimplified raddr with
  | K7R_RID(base,ofs) -> 
      sprintf "%s(%s)" 
	      (offsetToString ofs) 
	      (k7rintregToString base)
  | K7R_SID(idx,scalar,ofs) -> 
      sprintf "%s(,%s,%d)" 
	      (offsetToString ofs) 
	      (k7rintregToString idx) 
	      scalar
  | K7R_RISID(base,index,scalar,ofs) -> 
      sprintf "%s(%s,%s%s)" 
	      (offsetToString ofs)
	      (k7rintregToString base) 
	      (k7rintregToString index)
	      (scalarToString scalar)

let k7rbranchtargetToString (K7R_BTarget_Named name) = name

let k7rinstrToArgstrings = function
  | K7R_SimdLoadStoreBarrier	 -> []
  | K7R_SimdPromiseCellSize _    -> []
  | K7R_IntLoadMem(s,d) 	 -> [k7memopToString s; k7rintregToString d]
  | K7R_IntStoreMem(s,d)	 -> [k7rintregToString s; k7memopToString d]

  | K7R_IntLoadEA(s,d) 		 -> [k7raddrToString s; k7rintregToString d]
  | K7R_IntUnaryOp(op,sd)	 -> k7intunaryopToArgstrings op @
				    [k7rintregToString sd]
  | K7R_IntUnaryOpMem(op,sd)	 -> k7intunaryopToArgstrings op @
				    [k7memopToString sd]
  | K7R_IntCpyUnaryOp(op,s,d)    -> k7intcpyunaryopToArgstrings op @
				    [k7rintregToString s; k7rintregToString d]
  | K7R_IntBinOp(_,s,sd) 	 -> [k7rintregToString s; k7rintregToString sd]
  | K7R_IntBinOpMem(_,s,sd) 	 -> [k7memopToString s; k7rintregToString sd]
  | K7R_SimdLoad(_,s,d) 	 -> [k7raddrToString s; k7rmmxregToString d]
  | K7R_SimdStore(s,_,d) 	 -> [k7rmmxregToString s; k7raddrToString d]
  | K7R_SimdSpill(s,d) 		 -> [k7rmmxregToString s; 
				     k7mmxstackcellToString d]
  | K7R_SimdUnaryOp(op,sd) 	 -> [k7simdunaryopToArgstring op; 
				     k7rmmxregToString sd]
  | K7R_SimdCpyUnaryOp(_,s,d)    -> [k7rmmxregToString s; k7rmmxregToString d]
  | K7R_SimdCpyUnaryOpMem(_,s,d) -> [k7memopToString s; k7rmmxregToString d]
  | K7R_SimdBinOp(_,s,sd) 	 -> [k7rmmxregToString s; k7rmmxregToString sd]
  | K7R_SimdBinOpMem(_,s,sd) 	 -> [k7memopToString s; k7rmmxregToString sd]
  | K7R_Jump d			 -> [k7rbranchtargetToString d]
  | K7R_CondBranch(cond,d)	 -> [k7rbranchtargetToString d]
  | K7R_Ret			 -> []
  | K7R_FEMMS			 -> []
  | K7R_Label _ 		 -> failwith "k7rinstrToArgstrings: label!"

let k7rinstrToInstrstring = function
  | K7R_SimdLoadStoreBarrier	  -> "/* simd data load/store barrier */"
  | K7R_SimdPromiseCellSize i     -> Printf.sprintf 
					"/* promise simd cell size = %d */"
					(k7operandsizeToInteger i)
  | K7R_IntLoadMem _		  -> "movl"
  | K7R_IntStoreMem _ 		  -> "movl"
  | K7R_IntLoadEA _		  -> "leal"
  | K7R_IntUnaryOp(op,_) 	  -> k7intunaryopToString op
  | K7R_IntUnaryOpMem(op,_) 	  -> k7intunaryopToString op
  | K7R_IntCpyUnaryOp(op,_,_)     -> k7intcpyunaryopToString op
  | K7R_IntBinOp(op,_,_)	  -> k7intbinopToString op
  | K7R_IntBinOpMem(op,_,_)	  -> k7intbinopToString op
  | K7R_SimdLoad(size,_,_)	  -> k7operandsizeToMovstring size
  | K7R_SimdStore(_,size,_)	  -> k7operandsizeToMovstring size
  | K7R_SimdSpill _ 		  -> k7operandsizeToMovstring !mmx_reg_size
  | K7R_SimdUnaryOp(op,_) 	  -> k7simdunaryopToString op
  | K7R_SimdCpyUnaryOp(op,_,_)    -> k7simdcpyunaryopToString op
  | K7R_SimdCpyUnaryOpMem(op,_,_) -> k7simdcpyunaryopmemToString op
  | K7R_SimdBinOp(op,_,_)	  -> k7simdbinopToString op
  | K7R_SimdBinOpMem(op,s,_)	  -> k7simdbinopToString op
  | K7R_Jump _			  -> "jmp"
  | K7R_CondBranch(cond,_)	  -> "j" ^ k7branchconditionToString cond
  | K7R_Ret			  -> "ret"
  | K7R_FEMMS			  -> "femms"
  | K7R_Label _			  -> failwith "k7rinstrToInstrstring: label!"

let k7rinstrToString = function
  | K7R_Label lbl -> sprintf "\t.p2align 4,,7\n%s:" lbl
  | i ->
      "\t" ^ k7rinstrToInstrstring i ^ 
	" " ^ (stringlistToString ", " (k7rinstrToArgstrings i))

(* Warning: this function produces side-effects. *)
let k7rinstrInitStackPointerAdjustment value = 
  stackpointer_adjustment := value

(* Warning: this function produces side-effects. *)
(* This function detects some instructions that usually would keep us from
 * omitting the frame-pointer. Since we *always* omit the frame-pointer,
 * this function raises an exception in these situations. *)
let k7rinstrAdaptStackPointerAdjustment = function
  | K7R_IntUnaryOp(K7_IPush,_) ->
      stackpointer_adjustment := !stackpointer_adjustment + 4
  | K7R_IntUnaryOp(K7_IPop,_)  ->
      stackpointer_adjustment := !stackpointer_adjustment - 4
  | K7R_IntUnaryOp(K7_IAddImm i,sd) when sd = k7rintreg_stackpointer ->
      stackpointer_adjustment := !stackpointer_adjustment - i
  | K7R_IntUnaryOp(K7_ISubImm i,sd) when sd = k7rintreg_stackpointer ->
      stackpointer_adjustment := !stackpointer_adjustment + i
  | K7R_SimdPromiseCellSize s ->
      mmx_reg_size := s
  | instr ->
      if k7rinstrUsesK7rintreg k7rintreg_stackpointer instr then
	failwith "k7rinstrAdaptStackPointerAdjustment: attempt to access esp!"
      else
	()
