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


type k7stack = K7_INTStack | K7_MMXStack
and k7memop =
    K7_MConst of string
  | K7_MVar of string
  | K7_MFunArg of int
  | K7_MStackCell of k7stack * int
and k7intunaryop =
    K7_IPush
  | K7_IPop
  | K7_INegate
  | K7_IClear
  | K7_IInc
  | K7_IDec
  | K7_IAddImm of int
  | K7_ISubImm of int
  | K7_IShlImm of int
  | K7_IShrImm of int
  | K7_ILoadValue of int
and k7intcpyunaryop = K7_ICopy | K7_IMulImm of int
and k7intbinop = K7_IAdd | K7_ISub
and k7simdunaryop =
    K7_FPChs of VSimdBasics.vsimdpos
  | K7_FPMulConst of Number.number * Number.number
and k7simdcpyunaryop = K7_FPId | K7_FPSwap
and k7simdbinop =
    K7_FPAdd
  | K7_FPSub
  | K7_FPSubR
  | K7_FPMul
  | K7_FPPPAcc
  | K7_FPNNAcc
  | K7_FPNPAcc
  | K7_UnpckLo
  | K7_UnpckHi
and k7operandsize = K7_DWord | K7_QWord
and k7branchcondition =
    K7_BCond_NotZero
  | K7_BCond_Zero
  | K7_BCond_GreaterZero
  | K7_BCond_EqualZero
and k7vaddr =
    K7V_RID of VSimdBasics.vintreg * int
  | K7V_SID of VSimdBasics.vintreg * int * int
  | K7V_RISID of VSimdBasics.vintreg * VSimdBasics.vintreg * int * int
and k7vbranchtarget = K7V_BTarget_Named of string
and k7vinstr =
    K7V_IntLoadMem of k7memop * VSimdBasics.vintreg
  | K7V_IntStoreMem of VSimdBasics.vintreg * k7memop
  | K7V_IntLoadEA of k7vaddr * VSimdBasics.vintreg
  | K7V_IntUnaryOp of k7intunaryop * VSimdBasics.vintreg
  | K7V_IntUnaryOpMem of k7intunaryop * k7memop
  | K7V_IntCpyUnaryOp of k7intcpyunaryop * VSimdBasics.vintreg *
      VSimdBasics.vintreg
  | K7V_IntBinOp of k7intbinop * VSimdBasics.vintreg * VSimdBasics.vintreg
  | K7V_IntBinOpMem of k7intbinop * k7memop * VSimdBasics.vintreg
  | K7V_SimdLoad of k7operandsize * k7vaddr * VSimdBasics.vsimdreg
  | K7V_SimdStore of VSimdBasics.vsimdreg * k7operandsize * k7vaddr
  | K7V_SimdUnaryOp of k7simdunaryop * VSimdBasics.vsimdreg
  | K7V_SimdCpyUnaryOp of k7simdcpyunaryop * VSimdBasics.vsimdreg *
      VSimdBasics.vsimdreg
  | K7V_SimdCpyUnaryOpMem of k7simdcpyunaryop * k7memop *
      VSimdBasics.vsimdreg
  | K7V_SimdBinOp of k7simdbinop * VSimdBasics.vsimdreg *
      VSimdBasics.vsimdreg
  | K7V_SimdBinOpMem of k7simdbinop * k7memop * VSimdBasics.vsimdreg
  | K7V_SimdLoadStoreBarrier
  | K7V_RefInts of VSimdBasics.vintreg list
  | K7V_Label of string
  | K7V_Jump of k7vbranchtarget
  | K7V_CondBranch of k7branchcondition * k7vbranchtarget
  | K7V_SimdPromiseCellSize of k7operandsize
and k7rmmxreg = K7R_MMXReg of string
and k7rintreg = K7R_IntReg of string

val toK7rintreg : string -> k7rintreg
val toK7rmmxreg : string -> k7rmmxreg
val k7vFlops : k7vinstr list -> (int * int)

module RIntRegMap :
  sig
    type key = k7rintreg
    and (+'a) t
    val empty : 'a t
    val add : key -> 'a -> 'a t -> 'a t
    val find : key -> 'a t -> 'a
    val remove : key -> 'a t -> 'a t
    val mem : key -> 'a t -> bool
    val iter : (key -> 'a -> unit) -> 'a t -> unit
    val map : ('a -> 'b) -> 'a t -> 'b t
    val mapi : (key -> 'a -> 'b) -> 'a t -> 'b t
    val fold : (key -> 'a -> 'b -> 'b) -> 'a t -> 'b -> 'b
  end

module RSimdRegMap :
  sig
    type key = k7rmmxreg
    and (+'a) t
    val empty : 'a t
    val add : key -> 'a -> 'a t -> 'a t
    val find : key -> 'a t -> 'a
    val remove : key -> 'a t -> 'a t
    val mem : key -> 'a t -> bool
    val iter : (key -> 'a -> unit) -> 'a t -> unit
    val map : ('a -> 'b) -> 'a t -> 'b t
    val mapi : (key -> 'a -> 'b) -> 'a t -> 'b t
    val fold : (key -> 'a -> 'b -> 'b) -> 'a t -> 'b -> 'b
  end

type k7raddr =
    K7R_RID of k7rintreg * int
  | K7R_SID of k7rintreg * int * int
  | K7R_RISID of k7rintreg * k7rintreg * int * int
and k7rbranchtarget = K7R_BTarget_Named of string
and k7rinstr =
    K7R_IntLoadMem of k7memop * k7rintreg
  | K7R_IntStoreMem of k7rintreg * k7memop
  | K7R_IntLoadEA of k7raddr * k7rintreg
  | K7R_IntUnaryOp of k7intunaryop * k7rintreg
  | K7R_IntUnaryOpMem of k7intunaryop * k7memop
  | K7R_IntCpyUnaryOp of k7intcpyunaryop * k7rintreg * k7rintreg
  | K7R_IntBinOp of k7intbinop * k7rintreg * k7rintreg
  | K7R_IntBinOpMem of k7intbinop * k7memop * k7rintreg
  | K7R_SimdLoad of k7operandsize * k7raddr * k7rmmxreg
  | K7R_SimdStore of k7rmmxreg * k7operandsize * k7raddr
  | K7R_SimdSpill of k7rmmxreg * int
  | K7R_SimdUnaryOp of k7simdunaryop * k7rmmxreg
  | K7R_SimdCpyUnaryOp of k7simdcpyunaryop * k7rmmxreg * k7rmmxreg
  | K7R_SimdCpyUnaryOpMem of k7simdcpyunaryop * k7memop * k7rmmxreg
  | K7R_SimdBinOp of k7simdbinop * k7rmmxreg * k7rmmxreg
  | K7R_SimdBinOpMem of k7simdbinop * k7memop * k7rmmxreg
  | K7R_SimdLoadStoreBarrier
  | K7R_Label of string
  | K7R_Jump of k7rbranchtarget
  | K7R_CondBranch of k7branchcondition * k7rbranchtarget
  | K7R_FEMMS
  | K7R_Ret
  | K7R_SimdPromiseCellSize of k7operandsize

val k7rmmxregs_names : string list
val k7rintregs_names : string list
val k7rintregs_calleesaved_names : string list
val k7rintreg_stackpointer_name : string
val k7rmmxregs : k7rmmxreg list
val k7rintregs : k7rintreg list
val k7rintregs_calleesaved : k7rintreg list
val k7rintreg_stackpointer : k7rintreg
val retval : k7rintreg

val k7operandsizeToString : k7operandsize -> string
val k7operandsizeToInteger : k7operandsize -> int

val k7simdbinopIsParallel : k7simdbinop -> bool
val k7simdbinopFlops : k7simdbinop -> int * int
val k7simdbinopToParallel : k7simdbinop -> k7simdbinop

val vsimdunaryopToK7simdcpyunaryop :
  VSimdBasics.vsimdunaryop -> k7simdcpyunaryop

val vsimdbinopToK7simdbinop : VSimdBasics.vsimdbinop -> k7simdbinop

val k7vaddrToK7raddr :
  (VSimdBasics.vintreg -> k7rintreg) -> k7vaddr -> k7raddr

val k7vaddrToSimplified : k7vaddr -> k7vaddr
val k7vaddrToVintregs : k7vaddr -> VSimdBasics.vintreg list

val k7vbranchtargetToVintregs : k7vbranchtarget -> 'a list

val k7vinstrToSrcvregs :
  k7vinstr -> VSimdBasics.vsimdreg list * VSimdBasics.vintreg list

val k7vinstrToDstvregs :
  k7vinstr -> VSimdBasics.vsimdreg list * VSimdBasics.vintreg list

val k7vinstrToVregs :
  k7vinstr ->
  (VSimdBasics.vsimdreg list * VSimdBasics.vintreg list) *
  (VSimdBasics.vsimdreg list * VSimdBasics.vintreg list) *
  (VSimdBasics.vsimdreg list * VSimdBasics.vintreg list)

val k7raddrToK7rintregs : k7raddr -> k7rintreg list

val k7raddrToSimplified : k7raddr -> k7raddr

val k7rinstrToSrck7rregs : k7rinstr -> k7rmmxreg list * k7rintreg list
val k7rinstrToDstk7rregs : k7rinstr -> k7rmmxreg list * k7rintreg list

val k7rinstrIsMMX : k7rinstr -> bool

val k7rinstrReadsK7rreg :
  (k7rmmxreg list * k7rintreg list -> 'a list) -> 'a -> k7rinstr -> bool
val k7rinstrWritesK7rreg :
  (k7rmmxreg list * k7rintreg list -> 'a list) -> 'a -> k7rinstr -> bool

val k7rinstrReadsK7rmmxreg : k7rmmxreg -> k7rinstr -> bool
val k7rinstrReadsK7rintreg : k7rintreg -> k7rinstr -> bool

val k7rinstrWritesK7rmmxreg : k7rmmxreg -> k7rinstr -> bool
val k7rinstrWritesK7rintreg : k7rintreg -> k7rinstr -> bool

val k7rinstrUsesK7rmmxreg : k7rmmxreg -> k7rinstr -> bool
val k7rinstrUsesK7rintreg : k7rintreg -> k7rinstr -> bool
