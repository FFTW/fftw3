(*
 * Copyright (c) 1997-1999 Massachusetts Institute of Technology
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

(* Here, we take a schedule (produced by Vschedule) ordering a
   sequence of instructions, and produce an annotated schedule.  The
   annotated schedule has the same ordering as the original schedule,
   but is additionally partitioned into nested blocks of temporary
   variables.  The partitioning is computed via a heuristic algorithm.

   The blocking allows the C code that we generate to consist of
   nested blocks that help communicate variable lifetimes to the
   compiler. *)

(* In the standard distribution of FFTW 2.1.3 there is a module called
   'Asched'. This module ('VAnnotatedScheduler') only slightly differs
   from Asched. It has been adapted to deal with structures defined in
   module VSimdBasics. *)
   
open Util
open VScheduler
open VSimdBasics
open VSimdInstrOperandSet

type vannotated_schedule = 
    VAnnotate of 
	VSimdInstrOperandSet.t * 
	VSimdInstrOperandSet.t * 
	VSimdInstrOperandSet.t *
	int * 
	vaschedule
and vaschedule = 
  | VADone
  | VAInstr of vsimdinstr
  | VASeq of vannotated_schedule * vannotated_schedule


(* find all variables used inside a scheduling unit *)
let find_block_vars = 
  let rec find_block_vars' set0 = function
    | VDone     -> set0
    | VInstr i  -> List.fold_right add (vsimdinstrToSrcoperands i) set0
    | VPar ps   -> List.fold_left find_block_vars' set0 ps 
    | VSeq(a,b) -> find_block_vars' (find_block_vars' set0 a) b
  in find_block_vars' empty

let overlap xs zs = cardinal (inter xs zs)

(* reorder a list of schedules so as to maximize overlap of variables *)
let reorder l =
  let rec loop = function
    | [] -> []
    | (a,va) :: b ->
	let c  = List.map (fun (a, x) -> ((a, x), overlap va x)) b in
	let c' = Sort.list (fun (_, a) (_, b) -> a > b) c in
	let b' = List.map fst c' in	
	  a :: (loop b') in

  let l' = List.map (fun x -> x, find_block_vars x) l in
  (* start with smallest block --- does this matter ? *)
  match l' with
    | [] -> []
    | _ ->  
        let m = optionToValue (minimize (fun (_, x) -> cardinal x) l') in
        let l'' = Util.remove m l' in
          loop (m :: l'')


let rec rewrite_declarations force_decls (VAnnotate(_,_,declared,_,what)) =
  let m = !Magic.number_of_variables in

  let declare_it declared =
    if force_decls || cardinal declared >= m then
      (empty, declared)
    else
      (declared, empty)

  in match what with
    | VADone -> 
	VAnnotate(empty, empty, empty, 0, what)
    | VAInstr i -> 
        let (u, d) = declare_it declared in
          VAnnotate(empty, u, d, 0, what)
    | VASeq(a,b) ->
        let ma = rewrite_declarations false a
        and mb = rewrite_declarations false b
        in let VAnnotate (_, ua, _, _, _) = ma
        and VAnnotate (_, ub, _, _, _) = mb
        in let (u, d) = declare_it (union declared (union ua ub))
        in VAnnotate(empty, u, d, 0, VASeq(ma,mb))


let annotate schedule =
  let m = !Magic.number_of_variables in

  let rec really_analyze live_at_end = function
    | VDone -> 
	VAnnotate(live_at_end, empty, empty, 0, VADone)
    | VInstr i -> 
	let dsts = vsimdinstrToDstoperands i 
 	and srcs = vsimdinstrToSrcoperands i in
	let dsts' = List.fold_right add dsts empty 
	and srcs' = List.fold_right add srcs live_at_end in
	  VAnnotate(diff srcs' dsts', dsts', empty, 0, VAInstr i)
(*
	   VAnnotate(remove dst (List.fold_right add srcs live_at_end),
		     singleton dst, 
		     empty,
		     0,
		     VAInstr i))
*)

    | VSeq(a,b) ->
	let ab = analyze live_at_end b in
	let VAnnotate(live_at_begin_b,defined_b,_,depth_a,_) = ab in
	let aa = analyze live_at_begin_b a in
	let VAnnotate(live_at_begin_a,defined_a,_,depth_b,_) = aa in
	let defined = filter vsimdinstroperand_is_temporary 
			     (union defined_a defined_b) in
	let declarable = diff defined live_at_end in
	let undeclarable = diff defined declarable 
	and maxdepth = max depth_a depth_b in
	VAnnotate(live_at_begin_a, undeclarable, declarable, 
		  cardinal declarable + maxdepth,
		  VASeq(aa,ab))
    | _ -> failwith "really_analyze"

  (* the pick_best machinery is currently unused. Too slow, and
     I don't know what `best' means *)
  and analyze l =
    let pick_best w =
      optionToValue 
	(minimize
	  (function VAnnotate(_,_,_,depth,_) -> depth)
	  (List.map (really_analyze l) w))
    in function
      |	VSeq (a, VDone) -> analyze l a
      |	VSeq (VDone, a) -> analyze l a

       (* try to balance nested Par blocks *)
      |	VPar [a] -> pick_best [a]
      |	VPar l -> 
	  let n2 = (List.length l) / 2 in
	  let rec loop n a b =
	    if n = 0 then
	      (List.rev b, a)
	    else
	      match a with
		[] -> failwith "loop"
	      |	x :: y -> loop (n - 1) y (x :: b)
	  in let (a, b) = loop n2 (reorder l) []
	  in pick_best [VSeq (VPar a, VPar b)]

      | x -> pick_best [x]

  in rewrite_declarations true (analyze VSimdInstrOperandSet.empty schedule)


let annotatedscheduleToVsimdinstrs =
  let rec annotateToInstrs xs (VAnnotate(_,_,_,_,asch)) = aschToInstrs xs asch
  and aschToInstrs xs = function
    | VADone       -> xs
    | VAInstr x    -> x::xs
    | VASeq(s0,s1) -> annotateToInstrs (annotateToInstrs xs s1) s0
  in annotateToInstrs []

