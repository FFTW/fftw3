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


open Number				(* Arbitrary Precision Number Type   *)
open Variable				(* Def.s of Variables and Arrays     *)
open Printf				(* exports sprintf 		     *)
open VFpBasics				(* Def of vfpreg and vfpinstr types  *)
open Util				(* listToString 		     *)

let vfpregToString (V_FPReg idx) = sprintf "vfpreg(%d)" idx

let vfpregsToString vs = "[" ^ listToString vfpregToString "," vs ^ "]"

let vfpsummandToString = function
  | V_FPPlus x  -> "+" ^ (vfpregToString x)
  | V_FPMinus x -> "-" ^ (vfpregToString x)

let vfpsummandsToString vs = "[" ^ listToString vfpsummandToString "," vs ^ "]"

let vfpaccessToString = function
  | V_FPRealOfComplex -> "realpartOfComplex"
  | V_FPImagOfComplex -> "imagpartOfComplex"
  | V_FPReal	      -> "real"

let vfpunaryopToString = function
  | V_FPMulConst n -> sprintf "mulconst(%s)" (Number.to_string n)
  | V_FPNegate	   -> "negate"
  | V_FPId	   -> "fp_copy"

let vfpbinopToString = function
  | V_FPAdd -> "add"
  | V_FPSub -> "sub"
  | V_FPMul -> "mul"

let vfpinstrToString = function
  | V_FPLoad(s_access, s_array, s_elementindex, d_reg) -> 
      sprintf "ld.%s.fp(%s[%d],%s)"
	      (vfpaccessToString s_access)
	      (arrayToString s_array)
	      s_elementindex
	      (vfpregToString d_reg)
  | V_FPStore(s_reg, d_access, d_array, d_elementindex) ->
      sprintf "st.%s.fp(%s,%s[%d])"
	      (vfpaccessToString d_access)
	      (vfpregToString s_reg)
	      (arrayToString d_array)
	      d_elementindex
  | V_FPUnaryOp(op,s,d) ->
      sprintf "%s.fp(%s,%s)"
	      (vfpunaryopToString op)
	      (vfpregToString s)
	      (vfpregToString d)
  | V_FPBinOp(op,s1,s2,d) ->
      sprintf "%s.fp(%s,%s,%s)"
	      (vfpbinopToString op)
	      (vfpregToString s1)
	      (vfpregToString s2)
	      (vfpregToString d)
  | V_FPAddL(s_regs, d_reg) ->
      sprintf "addL.fp(%s,%s)"
	      (vfpsummandsToString s_regs)
	      (vfpregToString d_reg)
