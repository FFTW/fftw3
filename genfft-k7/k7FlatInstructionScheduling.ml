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

(* This module includes the instruction scheduler for basic blocks. *)

open Id
open List
open Util
open K7Basics
open K7InstructionSchedulingBasics
open K7ExecutionModel

(****************************************************************************)

type dependency =			(* INSTRUCTION DEPENDENCY ***********)
  | RAW of Id.t * Id.t * int		(*   Read after Write (w/Latency)   *)
  | WAR of Id.t * Id.t			(*   Write after Read 		    *)
  | WAW of Id.t * Id.t 			(*   Write after Write		    *)

let depToSucc    = function RAW(r,_,_) -> r | WAR(w,_) -> w | WAW(w,_) -> w
let depToLatency = function RAW(_,_,l) -> l | WAR _    -> 0 | WAW _    -> 0

(****************************************************************************)

let calc_forward_dependencies instrs = 
  let calc_fw_deps (idmap,deps,lastreads,lastwrite) instr =
    let id = Id.makeNew () in

    let addDepsR r_res = match resmap_find r_res lastwrite with	     (* RAW *)
      | Some (w_id,w_latency) -> idmap_addE w_id (RAW(id,w_id,w_latency))
      | None -> return
    and addDepsW (w_res,_) deps = 			     (* WAW and WAR *)
      fold_right (fun r_id -> idmap_addE r_id (WAR(id,r_id)))
		 (resmap_findE w_res lastreads) 
		 (match resmap_find w_res lastwrite with
      		    | Some (w_id,_) -> idmap_addE w_id (WAW(id,w_id)) deps
      		    | None -> deps) in

    let (rs,ws)    = (k7rinstrToSrck7rops instr, k7rinstrToDstk7rops instr) in
    let lastreads' = fold_right (resmap_addE' id) rs lastreads in
      (IdMap.add id instr idmap,
       fold_right addDepsW ws (fold_right addDepsR rs (IdMap.add id [] deps)),
       fold_right (fun (res,_) -> ResMap.add res []) ws lastreads',
       fold_right (fun (res,lat) -> ResMap.add res (id,lat)) ws lastwrite) in

  let s0 = (IdMap.empty, IdMap.empty, ResMap.empty, ResMap.empty) in
  let (idmap,fw_deps,_,_) = fold_left calc_fw_deps s0 instrs in
    ((fun id -> IdMap.find id idmap), fw_deps)

(****************************************************************************)

let calc_backward_dependencies_count fw_deps =
  IdMap.fold (fun _ -> fold_right (fun dep -> idmap_inc (depToSucc dep)))
	      fw_deps 
	      (IdMap.map toZero fw_deps)

(****************************************************************************)

let calc_critical_path_lengths fw_deps bw_deps_cnt = 
  let fold_start_instrs id cnt = if cnt = 0 then cons id else return in
  let startinstrs = IdMap.fold fold_start_instrs bw_deps_cnt [] in

  let rec calc_cplen' id cplen = 
    if idmap_exists id cplen then cplen else calc_cplen'' id cplen

  and calc_cplen'' id cplen = match IdMap.find id fw_deps with
    | [] -> IdMap.add id 0 cplen
    | deps ->
        let cplen' = fold_right calc_cplen' (map depToSucc deps) cplen in
	let depToCPLen d = depToLatency d + IdMap.find (depToSucc d) cplen' in
          IdMap.add id (max_list (map depToCPLen deps)) cplen' in

  let cplens = fold_right calc_cplen' startinstrs IdMap.empty in
    fun id -> IdMap.find id cplens

(*****************************************************************************)

(* 'waiting' instrs are blocked until some other instrs have been issued. *)
type bwdepscnt_est = BWCnt_EST of int * int

(* a 'preready' instruction i needs to wait until t >= est(i) *)
type est_id = EST_Id of int * Id.t
module ESTIdSet = Set.Make(struct type t = est_id let compare = compare end)

(* 'ready' instructions may be issued at any time *)
type cp_id = CP_Id of int * Id.t


(* used when sorting a list of ready instructions (in descending order) *)
let cmp_cpid (CP_Id(x_cplen,_)) (CP_Id(y_cplen,_)) = compare y_cplen x_cplen

(****************************************************************************)

let flatinssched idmap fw_deps cplens =
  (* move instructions from 'preready' to 'ready' *)
  let rec add_new_ready t = function
    | (_, s) as pair when ESTIdSet.is_empty s -> pair
    | (ready,preready) as pair ->
        match ESTIdSet.min_elt preready with
	  | EST_Id(est,_) when t < est -> pair
	  | EST_Id(_,id) as min_el ->
	      let ready' = insert_list cmp_cpid (CP_Id(cplens id,id)) ready in
	        add_new_ready t (ready', ESTIdSet.remove min_el preready)

  (* adapt earliest starting time. decrement the number of bw deps of an
   * instruction. if it is zero, move the instr from waiting to preready. *)
  and adapt_est' t dep (preready,waiting) =
    let id = depToSucc dep in
    let (r_bwdepslen,r_est) = IdMap.find id waiting in
    let r_est' = max (t + (depToLatency dep)) r_est in
      if r_bwdepslen > 1 then	     (* # of other bw_deps of the successor *)
	(preready, IdMap.add id (pred r_bwdepslen,r_est') waiting)
      else
	(ESTIdSet.add (EST_Id(r_est',id)) preready, IdMap.remove id waiting)

  (* enforce forward dependencies for one instruction (starting at t) *)
  and adapt_est t id = fold_right (adapt_est' t) (IdMap.find id fw_deps)

  (* the actual instruction scheduling loop *)
  and flatinssched' t nr_freeslots instrs_in_t instrs
		    ((ready, preready) as ready_preready) waiting =

    let complete () = rev (instrs_in_t @ instrs)
    and start_new_cycle t' = 
      flatinssched' t' k7SlotsPerCycle [] (instrs_in_t @ instrs)
		    (add_new_ready t' ready_preready) waiting in

    match (ready,preready,waiting) with
      | ([],p,w) when ESTIdSet.is_empty p && w = IdMap.empty -> complete ()
      | ([],p,_) when not (ESTIdSet.is_empty p) -> start_new_cycle (succ t)
      | ([],_,_) -> failwith "flatinssched: inconsistent state!"
      | (_::_,_,_) when nr_freeslots = 0 -> start_new_cycle (succ t)
      | (_::_ as ready,_,_) ->
	  (* select an instr with the longest critical path that may be issued
	   * simultaneously with the ones already selected earlier. *)
	  let instrs_in_t' = map get3of3 instrs_in_t in
	  let instrMayIssueInThisCycle (CP_Id(_,id)) = 
	    k7rinstrsCanIssueInOneCycle (idmap id::instrs_in_t') in
	  match selectFirst instrMayIssueInThisCycle ready with
            | None -> start_new_cycle (succ t)
            | Some((CP_Id(cplen,id),ready')) ->
	        let preready',waiting' = adapt_est t id (preready,waiting) in
		let instrs_in_t' = (t,cplen,idmap id)::instrs_in_t in
	          flatinssched' t (pred nr_freeslots) instrs_in_t' instrs
				(add_new_ready t (ready',preready')) waiting'

 in flatinssched' 0 k7SlotsPerCycle [] []


(* EXPORTED FUNCTIONS *******************************************************)

let k7rinstrsToInstructionscheduled instrs = 
  let (idmap,fw_deps) = calc_forward_dependencies instrs in
  let bw_deps_cnt = calc_backward_dependencies_count fw_deps in
  let toCP = calc_critical_path_lengths fw_deps bw_deps_cnt in
  let foldReady id (n,_) = if n=0 then cons (CP_Id(toCP id,id)) else return in
  let all = IdMap.map (fun cnt -> (cnt, 0)) bw_deps_cnt in
  let ready = sort cmp_cpid (IdMap.fold foldReady all []) in
  let waiting = fold_right (fun (CP_Id(_,i)) -> IdMap.remove i) ready all in
    flatinssched idmap fw_deps toCP (ready, ESTIdSet.empty) waiting

