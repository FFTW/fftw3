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

(* In this module, vinstrs are translated to k7vinstrs. *)

open List
open Variable
open VFpBasics
open VSimdBasics
open K7Basics

(****************************************************************************)

(* basic unparsers *)
let fail_unparser msg _ = failwith msg

let fixedstride_complex_unparser stride base idx = function
  | None		   -> K7V_RID(base,idx*8*stride)
  | Some V_FPRealOfComplex -> K7V_RID(base,idx*8*stride + 0)
  | Some V_FPImagOfComplex -> K7V_RID(base,idx*8*stride + 4)
  | Some _ -> failwith "fixedstride_complex_unparser: unsupported!"

let unitstride_complex_unparser = fixedstride_complex_unparser 1

let strided_real_unparser (baseRe,oneRe) idx = function
  | Some V_FPReal -> K7V_RISID(baseRe,oneRe,idx,0)
  | _ -> failwith "strided_real_unparser: unsupported!"

let strided_realofcomplex_unparser_withoffset (baseRe,oneRe,ofs) idx = function
  | Some V_FPRealOfComplex -> K7V_RISID(baseRe,oneRe,idx+ofs,0)
  | _ -> failwith "strided_realofcomplex_unparser_withoffset: unsupported!"

let strided_imagofcomplex_unparser_withoffset (baseIm,oneIm,ofs) idx = function
  | Some V_FPImagOfComplex -> K7V_RISID(baseIm,oneIm,idx+ofs,0)
  | _ -> failwith "strided_imagofcomplex_unparser_withoffset: unsupported!"

let strided_real_split2_unparser (base_1,base_2,split,one) idx = function
  | Some V_FPReal when idx < split -> K7V_RISID(base_1,one,idx,0)
  | Some V_FPReal -> K7V_RISID(base_2,one,idx-split,0)
  | _ -> failwith "strided_real_split2_unparser: unsupported!"

let strided_dualreal_unparser (baseRe,oneRe) (baseIm,oneIm) idx = function
  | Some V_FPRealOfComplex -> K7V_RISID(baseRe,oneRe,idx,0)
  | Some V_FPImagOfComplex -> K7V_RISID(baseIm,oneIm,idx,0)
  | _ -> failwith "strided_dualreal_unparser: unsupported!"

let strided_complex_unparser (base,one) idx = function
  | None 		   -> K7V_RISID(base,one,idx,0)
  | Some V_FPRealOfComplex -> K7V_RISID(base,one,idx,0)
  | Some V_FPImagOfComplex -> K7V_RISID(base,one,idx,4)
  | _ -> failwith "strided_complex_unparser: unsupported!"

let strided_complex_split2_unparser (base_1,base_2,split,one) idx access =
  let base',idx' = if idx < split then (base_1,idx) else (base_2,idx-split) in
    strided_complex_unparser (base',one) idx' access

let strided_hc2hc_unparser_1 (baseRe,baseIm,one,n) idx = function
  | Some V_FPRealOfComplex -> K7V_RISID(baseRe,one,idx,0)
  | Some V_FPImagOfComplex -> K7V_RISID(baseIm,one,idx+1,0)
  | _ -> failwith "strided_hc2hc_unparser: unsupported!"

let strided_hc2hc_unparser_2 (baseRe,baseIm,one,n) idx = function
  | Some V_FPRealOfComplex -> K7V_RISID(baseRe,one,idx,0)
  | Some V_FPImagOfComplex -> K7V_RISID(baseIm,one,n-idx,0)
  | _ -> failwith "strided_hc2hc_unparser: unsupported!" 

(****************************************************************************)

(* combine three unparsers (for Input, Output, Twiddle) *)
let make_asm_unparser (in_instrs,  in_unparser) 
		      (out_instrs, out_unparser) 
		      (tw_instrs,  tw_unparser) 
		      context idx = function
  | Input   -> (in_instrs,  k7vaddrToSimplified (in_unparser idx context))
  | Output  -> (out_instrs, k7vaddrToSimplified (out_unparser idx context))
  | Twiddle -> (tw_instrs,  k7vaddrToSimplified (tw_unparser idx context))


let make_asm_unparser_notwiddle in_unparser' out_unparser' =
  make_asm_unparser in_unparser' 
	  	    out_unparser'
	  	    ([], fail_unparser "twiddle factors not supported!")

(****************************************************************************)

let vsimdunaryopToK7vinstrs (* constToVsimdreg *) s d = function
  | V_MulConst(n,m) ->
      [K7V_SimdCpyUnaryOp(K7_FPId,s,d); K7V_SimdUnaryOp(K7_FPMulConst(n,m),d)]
  | V_Chs p ->
      [K7V_SimdCpyUnaryOp(K7_FPId,s,d); K7V_SimdUnaryOp(K7_FPChs p, d)]
  | op ->
      [K7V_SimdCpyUnaryOp(vsimdunaryopToK7simdcpyunaryop op, s, d)]

(* map one vsimdinstr to the corresponding sequence of k7vinstrs *)
let vsimdinstrToK7vinstrs unparser = function
  | V_SimdLoadQ(arr,idx,dst) -> 
      let (instrs,addr) = unparser None idx arr in
        instrs @ [K7V_SimdLoad(K7_QWord,addr,dst)]
  | V_SimdStoreQ(src,arr,idx) -> 
      let (instrs,addr) = unparser None idx arr in 
	instrs @ [K7V_SimdStore(src,K7_QWord,addr)]
  | V_SimdLoadD(reim,arr,idx,V_Lo,dst) ->
      let (instrs,addr) = unparser (Some reim) idx arr in
        instrs @ [K7V_SimdLoad(K7_DWord,addr,dst)]
  | V_SimdStoreD(V_Lo,src,reim,arr,idx) ->
      let (instrs,addr) = unparser (Some reim) idx arr in 
	instrs @ [K7V_SimdStore(src,K7_DWord,addr)]
  | V_SimdLoadD _ | V_SimdStoreD _ ->
      failwith "vinstrToK7vinstrs: unsupported use of loadD/storeD!"
  | V_SimdUnaryOp(op,s,d) ->
      vsimdunaryopToK7vinstrs s d op
  | V_SimdBinOp(op,s1,s2,d) ->
      [K7V_SimdCpyUnaryOp(K7_FPId, s1, d); 
       K7V_SimdBinOp(vsimdbinopToK7simdbinop op, (if s1=s2 then d else s2), d)]


let rec insertSimdLoadStoreBarrier = function
  | [] -> failwith "insertSimdLoadStoreBarrier"
  | (K7V_SimdStore _)::xs as xxs -> K7V_SimdLoadStoreBarrier::xxs
  | x::xs -> x::(insertSimdLoadStoreBarrier xs)

(* map a list of vsimdinstrs to a list of k7vinstrs *)
let vsimdinstrsToK7vinstrs unparser (operandsize,instrs) =
  [K7V_SimdPromiseCellSize operandsize] @
  (insertSimdLoadStoreBarrier 
	(concat (map (vsimdinstrToK7vinstrs unparser) instrs)))
