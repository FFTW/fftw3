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

open List
open Util
open VFpBasics
open NonDetMonad

let fetchAlreadyNegatedM = fetchStateM
let storeAlreadyNegatedM = storeStateM

type balancingmode = 
  | Balance_Aggressively
  | Balance_Conservatively

let balancingmodeToString = function
  | Balance_Aggressively   -> "Balance_Aggressively"
  | Balance_Conservatively -> "Balance_Conservatively"


let isFaultyUnaryOp d instrs mode = function
  | V_FPId -> true
  | V_FPMulConst _ -> false
  | V_FPNegate when mode = Balance_Aggressively -> true
  | V_FPNegate ->
      (match filter (fun i -> mem d (vfpinstrToSrcregs i)) instrs with
	 | [] -> failwith "isFaultyUnaryOp: encountered dead instr!"
	 | [V_FPUnaryOp(V_FPNegate,_,_)] -> true
	 | _ -> false)

let isFaultyInstr instrs mode = function
  | V_FPAddL(xs,_)	-> for_all vfpsummandIsNegative xs
  | V_FPUnaryOp(op,_,d)	-> isFaultyUnaryOp d instrs mode op
  | V_FPLoad _		-> false
  | V_FPStore _		-> false
  | V_FPBinOp _		-> false

let propagateNegationOf1M x = function
  | V_FPLoad(_,_,_,d)  when d=x -> failM
  | V_FPStore(s,_,_,_) when s=x -> failM
  | V_FPUnaryOp(op,s,d) when s=x || d=x -> 
      (match op with
	 | V_FPMulConst n -> unitM (V_FPMulConst (Number.negate n))
	 | V_FPNegate     -> unitM V_FPId
	 | V_FPId 	  -> failM
      ) >>= fun op' -> unitM (V_FPUnaryOp(op',s,d))
  | V_FPBinOp(op,s1,s2,d) when d=x || s1=x || s2=x ->
      failwith "propagateNegationOf1M: V_FPBinOp not supported!"
  | V_FPAddL(xs,d) when d=x ->
      unitM (V_FPAddL(map vfpsummandToNegated xs,d))
  | V_FPAddL(xs,d) when mem x (map vfpsummandToVfpreg xs) ->
      unitM (V_FPAddL(map (function
			     | V_FPPlus a when a=x  -> V_FPMinus a
			     | V_FPMinus a when a=x -> V_FPPlus a
			     | summand -> summand) xs, d))
  | instr ->
      unitM instr

let propagateNegationOfM x instrs =
  fetchAlreadyNegatedM >>= fun already_negated ->
    negAssertM (mem x already_negated) >>
      storeAlreadyNegatedM (x::already_negated) >>
        mapM (propagateNegationOf1M x) instrs


let applyVfpregSubstInVfpreg (x,x') z = if z=x then x' else z

let applyVfpregSubstInVfpsummand subst = function
  | V_FPMinus x -> V_FPMinus (subst x)
  | V_FPPlus x  -> V_FPPlus (subst x)

let applyVfpregSubstInInstr subst = function
  | V_FPLoad(s_access,s_array,s_idx,d) ->
      V_FPLoad(s_access,s_array,s_idx,subst d)
  | V_FPStore(s,d_access,d_array,d_idx) -> 
      V_FPStore(subst s, d_access,d_array,d_idx)
  | V_FPUnaryOp(op,s,d) -> 
      V_FPUnaryOp(op,subst s,subst d)
  | V_FPBinOp(op,s1,s2,d) ->
      V_FPBinOp(op,subst s1,subst s2,subst d)
  | V_FPAddL(xs,d) ->
      V_FPAddL(map (applyVfpregSubstInVfpsummand subst) xs, subst d)


let repairInstrAndPropagateM mode (f_instr,instrs) = match f_instr with
  | V_FPUnaryOp(V_FPId,s,d) ->
      let subst_fn = applyVfpregSubstInVfpreg (s,d) in
        unitM (map (applyVfpregSubstInInstr subst_fn) instrs)

  | V_FPUnaryOp(V_FPNegate,s,d) ->
      if mode = Balance_Conservatively then
	selectFirstM (fun i -> mem d (vfpinstrToSrcregs i)) instrs >>= 
 	  (function
	     | (V_FPUnaryOp(V_FPNegate,_,d'), instrs') ->
		  unitM (V_FPUnaryOp(V_FPId,s,d')::instrs')
	     | _ ->
		  failwith "repairInstrAndPropagateM")
      else
	memberM [s;d] >>= fun operand ->
	  propagateNegationOfM operand instrs >>= fun instrs' ->
	    unitM (V_FPUnaryOp(V_FPId,s,d)::instrs')

  | V_FPAddL(xs,d) ->
      if mode = Balance_Conservatively then
	let d' = makeNewVfpreg () in
	  unitM (V_FPAddL(map vfpsummandToNegated xs,d')::
		   V_FPUnaryOp(V_FPNegate,d',d)::instrs)
      else
	(   ( fun _ ->
	        propagateNegationOfM d instrs >>= fun instrs' ->
	          unitM (V_FPAddL(map vfpsummandToNegated xs,d)::instrs')
	    )
        ||| ( fun _ ->
	        memberM xs >>= fun x ->
		  propagateNegationOfM (vfpsummandToVfpreg x) (f_instr::instrs)
	    )
        )
  | _ -> failwith "repairInstrAndPropagateM: cannot repair this instruction!"



let rec balanceM mode instrs =
  if not (exists (isFaultyInstr instrs mode) instrs) then
    unitM instrs
  else
    selectM instrs >>= fun (x,xs) ->
      posAssertM (isFaultyInstr xs mode x) >>= fun _ ->	(* don't use >> here *)
        repairInstrAndPropagateM mode (x,xs) >>=
	  balanceM mode

let balanceM' instrs =
  memberM [Balance_Aggressively; Balance_Conservatively] >>= fun mode ->
    balanceM mode instrs >>= fun instrs' ->
      let _ = Util.info (Printf.sprintf "finished balancing in mode %s" 
				(balancingmodeToString mode)) in
        unitM instrs'

(****************************************************************************)

let rec balance already_negated instrs = 
  if not (exists (isFaultyInstr instrs Balance_Aggressively) instrs) then
    let _ = Util.info "BalanceVfpinstrs.balance: no balancing done" in instrs
  else
    match runM balanceM' instrs already_negated with
      | None	     -> failwith "balance: unhandled faulty instruction"
      | Some instrs' -> instrs'

(****************************************************************************)

let expandaddlistVfpinstr = function
  | V_FPAddL([x1;x2],d) ->
     (match addlistHelper2 d (x1,x2) with
	| (instr, V_FPPlus _) -> instr
	| (_, V_FPMinus _)    -> failwith "expandaddlistVfpinstr")
  | instr -> 
      instr

(****************************************************************************)

let rec fullyexpandaddlistVfpinstr' instrs0 dst = function
  | [] -> failwith "fullyexpandaddlistVfpinstr: []"
  | [x']  -> (x', instrs0)
  | [x1';x2'] ->
      let (fp_instr,dst') = addlistHelper2 dst (x1',x2') in 
	(dst', fp_instr::instrs0)
  | xs ->
      let (xs_l, xs_r) = zip xs in
      let (dst_l, dst_r) = (makeNewVfpreg (), makeNewVfpreg ()) in
	let dst_l',instrs1 = fullyexpandaddlistVfpinstr' instrs0 dst_l xs_l in
	let dst_r',instrs2 = fullyexpandaddlistVfpinstr' instrs1 dst_r xs_r in
	  fullyexpandaddlistVfpinstr' instrs2 dst [dst_l'; dst_r']

let fullyexpandaddlistVfpinstr = function
  | V_FPAddL(xs,d) ->
      (match fullyexpandaddlistVfpinstr' [] d xs with
	 | V_FPPlus _, instrs -> instrs
	 | V_FPMinus _, _     -> failwith "fullyexpandaddlistVfpinstr")
  | instr ->
      [instr]
