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

(* This module includes code for the translation of effective addresses. *)


open List
open Util
open K7Basics
open VSimdBasics
open K7RegisterAllocationBasics
open K7Unparsing
open VSimdUnparsing

open NonDetMonad

let isMulExpr one (rreg,rregfileentry) = match rregfileentry with 
  | ITmpHoldsProduct(base,factor) when base=one -> Some(factor,rreg)
  | IVarHoldsProduct(base,_,_) when base=one -> Some(1,rreg)
  | IVarHoldsProduct(_,base,factor) when base=one -> Some(factor,rreg)
  | IHolds reg when reg = one 	       -> Some(1,rreg)
  | _ -> None 

let isNTimesOne n one (rreg,rregfileentry) = match rregfileentry with 
  | IHolds reg when reg=one && n=1 	    -> true
  | ITmpHoldsProduct(base,k) when base=one && n=k -> true
  | IVarHoldsProduct(_,base,k) when base=one && n=k -> true
  | IVarHoldsProduct(base,_,_) when base=one && n=1 -> true
  | _ -> false


let find_leaM' one search_depth_limit n = 
  let getNewIntRegM () = 					(* LRU *)
    fetchStateM >>= fun riregfile0 ->
      selectM riregfile0 >>= fun ((reg,regfileentry),riregfile) ->
	posAssertM (riregfileentryIsFree regfileentry) >>
	  storeStateM riregfile >>
	    unitM reg in

  let availableConstantsM () =
    fetchStateM >>= fun riregfile ->
      memberM riregfile >>= fun reg_content ->
	optionToValueM (isMulExpr one reg_content) in

  let onestepM (x,x') z' = 			(* redundant solutions? *)
    disjM [(fun _ -> 
	      memberM [1;2;4;8] >>= fun s -> 
	        unitM (x*(s+1), K7R_IntLoadEA(K7R_RISID(x',x',s,0), z')));
	   (fun _ -> 
	      memberM [4;8] >>= fun s -> 
	        unitM (x*s, K7R_IntLoadEA(K7R_SID(x',s,0), z')));
	   (fun _ -> 
	      memberM [1;2;4;8] >>= fun s -> 
	        availableConstantsM () >>= fun (k,k') -> 
		  unitM (x*s+k, K7R_IntLoadEA(K7R_RISID(k',x',s,0), z')));
	   (fun _ -> 
	      memberM [2;4;8] >>= fun s -> 
		availableConstantsM () >>= fun (k,k') -> 
		  unitM (x+k*s, K7R_IntLoadEA(K7R_RISID(x',k',s,0), z')))] in


  let addNewMultipleOfOneM z reg =
    fetchStateM >>= fun riregfile0 ->
      negAssertM (optionIsSome (find_elem (isNTimesOne z one) riregfile0)) >>
	storeStateM (riregfile0 @ [(reg, ITmpHoldsProduct(one,z))]) in

  let rec nstepsM instrs (x,x') depth_limit = 
    if x = n then
      unitM (rev instrs, x')
    else 
      fetchStateM >>= fun riregfile ->
        match find_elem (isNTimesOne n one) riregfile with
	  | Some(rreg,_) -> unitM ([],rreg)
	  | None ->
	      posAssertM (depth_limit > 0) >>= fun _ ->
	        getNewIntRegM () >>= fun reg ->
	          onestepM (x,x') reg >>= fun (z,instr) ->
		    addNewMultipleOfOneM z reg >>
		      nstepsM (instr::instrs) (z,reg) (pred depth_limit) in


  availableConstantsM () >>= fun (k,one') ->
    posAssertM (k=1) >>= fun _ ->
      (   ( fun _ ->  (* first try: DFID with a sequence of LEA instructions *)
	      betweenM 0 search_depth_limit >>= nstepsM [] (1,one')
	  )
      ||| ( fun _ -> 	 (* alternative: use the expensive IMUL instruction *)
	      getNewIntRegM () >>= fun reg ->
		addNewMultipleOfOneM n reg >>
		  unitM ([K7R_IntCpyUnaryOp(K7_IMulImm n, one', reg)], reg)
	  )
      )


let find_leaM one search_depth_limit n = 
  find_leaM' one search_depth_limit n >>= fun (instrs',idx') ->
    fetchStateM >>= fun riregfile' ->
      unitM (instrs',idx',riregfile')


(* TRY TO SUBSTITUTE 'IMUL' WITH 'LEA' WHEREVER POSSIBLE *)
let loadea_factor mapInt riregfile = function
  | K7V_RID(base,ofs) ->
      ([], K7R_RID(mapInt base,ofs), riregfile)
  | K7V_SID(idx,scalar,offset) ->
      if mem scalar [1;2;4;8] then
        ([], K7R_SID(mapInt idx,scalar,offset), riregfile)
      else
	failwith "loadea_factor: got SID(i,_,_) with i not in [1;2;4;8]"
  | K7V_RISID(base,idx,scalar,offset) -> 
      let k = find (fun i -> scalar mod i = 0) [8;4;2;1] in
        (match runM (find_leaM idx !Magic.imul_to_lea_limit)
		    (scalar/k) 
		    riregfile with
	  | None -> failwith "loadea_factor: internal error"
	  | Some (instrs,idx',riregfile') -> 
	      (instrs, K7R_RISID(mapInt base,idx',k,offset), riregfile'))

