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

(* Unparsing of vsimd-instructions. Only useful during debugging. *)

open Util
open Number
open Variable
open VSimdBasics
open VFpUnparsing
open Printf

let vsimdregToString (V_SimdReg idx) = sprintf "vsimdreg(%d)" idx
let vintregToString  (V_IntReg idx)  = sprintf "vintreg(%d)" idx
let realimagToString = function RealPart -> "re" | ImagPart -> "im"
let vsimdposToString = function V_Lo -> "lo" | V_Hi -> "hi" | V_LoHi -> "lohi"
let vsimdposToChsconstnamestring p = "chs_" ^ vsimdposToString p

let vsimdfpunaryopToString = function
  | V_Id -> 
      "id"
  | V_Swap -> 
      "swap"
  | V_Chs p -> 
      vsimdposToChsconstnamestring p
  | V_MulConst(n,m) -> 
      sprintf "mulconst(%s,%s)" (Number.to_string n) (Number.to_string m)

let vsimdfpbinopToString = function
  | V_Add     -> "add"
  | V_Sub     -> "sub"
  | V_SubR    -> "subr"
  | V_Mul     -> "mul"
  | V_PPAcc   -> "ppacc"
  | V_NNAcc   -> "nnacc"
  | V_NPAcc   -> "npacc"
  | V_UnpckLo -> "unpcklo"
  | V_UnpckHi -> "unpckhi"

let vsimdinstrToString = function
  | V_SimdLoadD(s_access,s_array,s_elementindex,d_pos,d_reg) ->
      sprintf "ld.d(%s.%s[%d], %s.%s)"
	      (vfpaccessToString s_access)
	      (arrayToString s_array)
	      s_elementindex
	      (vsimdregToString d_reg)
	      (vsimdposToString d_pos)
  | V_SimdLoadQ(s_array,s_elementindex,d_reg) ->
      sprintf "ld.q(%s[%d], %s)"
	      (arrayToString s_array)
	      s_elementindex
	      (vsimdregToString d_reg)
  | V_SimdStoreD(s_pos,s_reg,d_access,d_array,d_elementindex) ->
      sprintf "st.d(%s.%s, %s.%s[%d])"
	      (vsimdregToString s_reg)
	      (vsimdposToString s_pos)
	      (vfpaccessToString d_access)
	      (arrayToString d_array)
	      d_elementindex
  | V_SimdStoreQ(s_reg,d_array,d_elementindex) ->
      sprintf "st.q(%s, %s[%d])"
	      (vsimdregToString s_reg)
	      (arrayToString d_array)
	      d_elementindex
  | V_SimdUnaryOp(unary_op,s_reg,d_reg) ->
      sprintf "unaryOp(%s, %s, %s)"
	      (vsimdfpunaryopToString unary_op)
	      (vsimdregToString s_reg)
	      (vsimdregToString d_reg)
  | V_SimdBinOp(binary_op,s1_reg,s2_reg,d_reg) ->
      sprintf "binOp(%s, %s, %s, %s)"
	      (vsimdfpbinopToString binary_op)
	      (vsimdregToString s1_reg)
	      (vsimdregToString s2_reg)
	      (vsimdregToString d_reg)

let vsimdinstroperandToString = function
  | V_SimdDVar(access,arr,idx) -> 
      sprintf "%s.%s[%d]" (vfpaccessToString access) (arrayToString arr) idx
  | V_SimdQVar(arr,idx) -> 
      sprintf "%s[%d]" (arrayToString arr) idx
  | V_SimdTmp idx -> 
      sprintf "tmp%d" idx
  | V_SimdNumConst(c_lo,c_hi) -> 
      sprintf "simdConst(%s,%s)" (floatToString c_lo) (floatToString c_hi)
  | V_SimdNConst(name_str) -> 
      sprintf "namedConst(%s)" name_str

