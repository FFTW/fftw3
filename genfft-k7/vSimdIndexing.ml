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

(* This module contains code for hashing vsimdinstrs.  This feature is
 * used extensively in the code optimizer (VK7Optimization) to speedup
 * the code-optimization process. 					
 *)

open VSimdBasics

(* for hashing purposes *)
type vsimdinstrcategory = 
  | VSIC_Load
  | VSIC_Store
  | VSIC_UnaryMulConst
  | VSIC_UnaryCopy
  | VSIC_UnarySwap
  | VSIC_UnaryChsLo
  | VSIC_UnaryChsHi
  | VSIC_UnaryChsLoHi
  | VSIC_BinAdd
  | VSIC_BinSub
  | VSIC_BinMul
  | VSIC_BinPPAcc
  | VSIC_BinNNAcc
  | VSIC_BinNPAcc
  | VSIC_BinUnpckLo
  | VSIC_BinUnpckHi

(* vsimdinstr category map *)
module VSICMap = Map.Make(struct 
			    type t = vsimdinstrcategory
			    let compare = compare
			  end)

let vsicmap_findE k m = try VSICMap.find k m with Not_found -> []
let vsicmap_addE k v m = VSICMap.add k (v::(vsicmap_findE k m)) m

(****************************************************************************)

let vsimdunaryopcategories_chs = 
  [
   VSIC_UnaryChsLo;
   VSIC_UnaryChsHi;
   VSIC_UnaryChsLoHi;
  ]

let vsimdunaryopcategories_nocopy = 
  VSIC_UnarySwap::VSIC_UnaryMulConst::vsimdunaryopcategories_chs

let vsimdunaryopcategories_all = 
  VSIC_UnaryCopy::vsimdunaryopcategories_nocopy

let vsimdbinopcategories_par = [VSIC_BinAdd; VSIC_BinSub; VSIC_BinMul]

let vsimdbinopcategories_all =
  vsimdbinopcategories_par @
  [
   VSIC_BinPPAcc;
   VSIC_BinNNAcc;
   VSIC_BinNPAcc;
   VSIC_BinUnpckLo;
   VSIC_BinUnpckHi;
  ]

let vsimdallcategories = 
  [VSIC_Load; VSIC_Store] @ 
  vsimdunaryopcategories_all @ 
  vsimdbinopcategories_all

(****************************************************************************)

let vsimdunaryopToCategory = function
  | V_Id	 -> VSIC_UnaryCopy
  | V_Swap	 -> VSIC_UnarySwap
  | V_Chs V_Lo	 -> VSIC_UnaryChsLo
  | V_Chs V_Hi	 -> VSIC_UnaryChsHi
  | V_Chs V_LoHi -> VSIC_UnaryChsLoHi
  | V_MulConst _ -> VSIC_UnaryMulConst

let vsimdbinopToCategory = function
  | V_PPAcc   -> VSIC_BinPPAcc
  | V_NNAcc   -> VSIC_BinNNAcc
  | V_NPAcc   -> VSIC_BinNPAcc
  | V_UnpckLo -> VSIC_BinUnpckLo
  | V_UnpckHi -> VSIC_BinUnpckHi
  | V_Sub     -> VSIC_BinSub
  | V_SubR    -> VSIC_BinSub
  | V_Add     -> VSIC_BinAdd
  | V_Mul     -> VSIC_BinMul

let vsimdinstrToCategory = function
  | V_SimdLoadD _	  -> VSIC_Load
  | V_SimdLoadQ _	  -> VSIC_Load
  | V_SimdStoreD _ 	  -> VSIC_Store
  | V_SimdStoreQ _ 	  -> VSIC_Store
  | V_SimdUnaryOp(op,_,_) -> vsimdunaryopToCategory op
  | V_SimdBinOp(op,_,_,_) -> vsimdbinopToCategory op

let vsimdinstrToCategories i = [vsimdinstrToCategory i]



