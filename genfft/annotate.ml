(*
 * Copyright (c) 1997-1999 Massachusetts Institute of Technology
 * Copyright (c) 2003 Matteo Frigo
 * Copyright (c) 2003 Massachusetts Institute of Technology
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
(* $Id: annotate.ml,v 1.18 2005-02-26 05:31:52 athena Exp $ *)

(* Here, we take a schedule (produced by schedule.ml) ordering a
   sequence of instructions, and produce an annotated schedule.  The
   annotated schedule has the same ordering as the original schedule,
   but is additionally partitioned into nested blocks of temporary
   variables.  The partitioning is computed via a heuristic algorithm.

   The blocking allows the C code that we generate to consist of
   nested blocks that help communicate variable lifetimes to the
   compiler. *)

(* $Id: annotate.ml,v 1.18 2005-02-26 05:31:52 athena Exp $ *)
open Schedule
open Expr
open Variable

type annotated_schedule = 
    Annotate of variable list * variable list * variable list * int * aschedule
and aschedule = 
    ADone
  | AInstr of assignment
  | ASeq of (annotated_schedule * annotated_schedule)

let addelem a set = if not (List.memq a set) then a :: set else set
let union l = 
  let f x = addelem x   (* let is source of polymorphism *)
  in List.fold_right f l

(* set difference a - b *)
let diff a b = List.filter (fun x -> not (List.memq x b)) a

let rec minimize f = function
    [] -> failwith "minimize"
  | [n] -> n
  | n :: rest ->
      let x = minimize f rest in
      if (f x) >= (f n) then n else x

(* find all variables used inside a scheduling unit *)
let rec find_block_vars = function
    Done -> []
  | (Instr (Assign (v, x))) -> v :: (find_vars x)
  | Par a -> List.flatten (List.map find_block_vars a)
  | Seq (a, b) -> (find_block_vars a) @ (find_block_vars b)

let uniq l = 
  List.fold_right (fun a b -> if List.memq a b then b else a :: b) l []

let has_related x = List.exists (Variable.same_class x)

let rec overlap a b = Util.count (fun y -> has_related y b) a

(* reorder a list of schedules so as to maximize overlap of variables *)
let reorder l =
  let rec loop = function
      [] -> []
    | (a, va) :: b ->
	let c = 
	  List.map 
	    (fun (a, x) -> ((a, x), (overlap va x, List.length x))) b in
	let c' =
	  Sort.list 
	    (fun (_, (a, la)) (_, (b, lb)) -> 
	      la < lb or a > b)
	    c in
	let b' = List.map (fun (a, _) -> a) c' in
	a :: (loop b') in
  let l' = List.map (fun x -> x, uniq (find_block_vars x)) l in
  (* start with smallest block --- does this matter ? *)
  match l' with
    [] -> []
  | _ ->  
      let m = minimize (fun (_, x) -> (List.length x)) l' in
      let l'' = Util.remove m l' in
      loop (m :: l'')

(* remove Par blocks *)
let rec linearize = function
  | Seq (a, Done) -> linearize a
  | Seq (Done, a) -> linearize a
  | Seq (a, b) -> Seq (linearize a, linearize b)

  (* try to balance nested Par blocks *)
  | Par [a] -> linearize a
  | Par l -> 
      let n2 = (List.length l) / 2 in
      let rec loop n a b =
	if n = 0 then
	  (List.rev b, a)
	else
	  match a with
	    [] -> failwith "loop"
	  | x :: y -> loop (n - 1) y (x :: b)
      in let (a, b) = loop n2 (reorder l) []
      in linearize (Seq (Par a, Par b))

  | x -> x 

let subset a b =
  List.for_all (fun x -> List.exists (fun y -> x == y) b) a

let use_same_vars (Assign (av, ax)) (Assign (bv, bx)) =
  is_temporary av &&
  is_temporary bv &&
  (let va = Expr.find_vars ax and vb = Expr.find_vars bx in
   subset va vb && subset vb va)

let store_to_same_class (Assign (av, ax)) (Assign (bv, bx)) =
  is_locative av &&
  is_locative bv &&
  Variable.same_class av bv

let loads_from_same_class (Assign (av, ax)) (Assign (bv, bx)) =
  match (ax, bx) with
    | (Load a), (Load b) when 
	Variable.is_locative a && Variable.is_locative b 
	-> Variable.same_class a b
    | _ -> false

(* extract instructions from schedule *)
let rec sched_to_ilist = function
  | Done -> []
  | Instr a -> [a]
  | Seq (a, b) -> (sched_to_ilist a) @ (sched_to_ilist b)
  | _ -> failwith "sched_to_ilist" (* Par blocks removed by linearize *)

let rec find_friends friendp insn friends foes = function
  | [] -> (friends, foes)
  | a :: b -> 
      if (a == insn) || (friendp a insn) then
	find_friends friendp insn (a :: friends) foes b
      else
	find_friends friendp insn friends (a :: foes) b

(* schedule all instructions in the equivalence class determined
   by friendp at the point where the last one
   is executed *)
let rec delay_friends friendp sched =
  let rec recur insns = function
    | Done -> (Done, insns)
    | Instr a ->
	let (friends, foes) = find_friends friendp a [] [] insns in
	(Schedule.sequentially friends), foes
    | Seq (a, b) ->
	let (b', insnsb) = recur insns b in
	let (a', insnsa) = recur insnsb a in
	(Seq (a', b')), insnsa
    | _ -> failwith "delay_friends"
  in match recur (sched_to_ilist sched) sched with
  | (s, []) -> s (* assert that all insns have been used *)
  | _ -> failwith "delay_friends"

(* schedule all instructions in the equivalence class determined
   by friendp at the point where the first one
   is executed *)
let rec anticipate_friends friendp sched =
  let rec recur insns = function
    | Done -> (Done, insns)
    | Instr a ->
	let (friends, foes) = find_friends friendp a [] [] insns in
	(Schedule.sequentially friends), foes
    | Seq (a, b) ->
	let (a', insnsa) = recur insns a in
	let (b', insnsb) = recur insnsa b in
	(Seq (a', b')), insnsb
    | _ -> failwith "anticipate_friends"
  in match recur (sched_to_ilist sched) sched with
  | (s, []) -> s (* assert that all insns have been used *)
  | _ -> failwith "anticipate_friends"

let pair_stores buddy_list sched =
  let rec find_buddy v = function
    | [] -> None
    | (even, odd) :: rest when v == even or v == odd -> Some (even, odd)
    | _ :: rest -> find_buddy v rest
  in
  let rec recur sched delayed_stores = match sched with
    | Done -> (sched, delayed_stores)
    | Instr (Assign (v, x)) ->
	begin
	  match find_buddy v buddy_list with
	    | None -> (sched, delayed_stores)
	    | Some (even, odd) -> 
		try 
		  let (vbuddy, xbuddy) = 
		    List.find (fun (v, _) -> v == even or v == odd)
		      delayed_stores
		  in 
		  let tmp = Variable.make_temporary () in
		  let a = Instr (Assign (tmp, x)) in
		  let pair1 = Instr (Assign (v, Times (NaN PAIR1, Load tmp)))
		  and pair2 = 
		    if (v == even) then
		      Instr (Assign (v, 
				     Times (NaN PAIR2,
					    Plus [Load tmp; xbuddy])))
		    else 
		      Instr (Assign (vbuddy, 
				     Times (NaN PAIR2,
					    Plus [xbuddy; Load tmp])))
		  in Seq (a, Seq(pair1, pair2)), delayed_stores
		with Not_found ->
		  let tmp = Variable.make_temporary () in
		    (Seq ((Instr (Assign (tmp, x))),
		          (Instr (Assign (v, Times (NaN PAIR1, Load tmp)))))),
		    (v, Load tmp) :: delayed_stores
	end
    | Seq (a, b) ->
	let (newa, delayed_stores) = recur a delayed_stores in
	let (newb, delayed_stores) = recur b delayed_stores in
	  (Seq (newa, newb), delayed_stores)
    | _ -> failwith "pair_stores"
  in let (sched, _) = recur sched [] in
    sched
  
  
let rec rewrite_declarations force_declarations 
    (Annotate (_, _, declared, _, what)) =
  let m = !Magic.number_of_variables in

  let declare_it declared =
    if (force_declarations or List.length declared >= m) then
      ([], declared)
    else
      (declared, [])

  in match what with
    ADone -> Annotate ([], [], [], 0, what)
  | AInstr i -> 
      let (u, d) = declare_it declared
      in Annotate ([], u, d, 0, what)
  | ASeq (a, b) ->
      let ma = rewrite_declarations false a
      and mb = rewrite_declarations false b
      in let Annotate (_, ua, _, _, _) = ma
      and Annotate (_, ub, _, _, _) = mb
      in let (u, d) = declare_it (declared @ ua @ ub)
      in Annotate ([], u, d, 0, ASeq (ma, mb))

let annotate list_of_pairable_stores schedule =
  let m = !Magic.number_of_variables in

  let rec analyze live_at_end = function
      Done -> Annotate (live_at_end, [], [], 0, ADone)
    | Instr i -> (match i with
	Assign (v, x) -> 
	  let vars = (find_vars x) in
	  Annotate (Util.remove v (union live_at_end vars), [v], [],
		    0, AInstr i))
    | Seq (a, b) ->
	let ab = analyze live_at_end b in
	let Annotate (live_at_begin_b, defined_b, _, depth_a, _) = ab in
	let aa = analyze live_at_begin_b a in
	let Annotate (live_at_begin_a, defined_a, _, depth_b, _) = aa in
	let defined = List.filter is_temporary (defined_a @ defined_b) in
	let declarable = diff defined live_at_end in
	let undeclarable = diff defined declarable 
	and maxdepth = max depth_a depth_b in
	Annotate (live_at_begin_a, undeclarable, declarable, 
		  List.length declarable + maxdepth,
		  ASeq (aa, ab))
    | _ -> failwith "really_analyze"

  in 
  let () = Util.info "begin annotate" in
  let x = linearize schedule in
  let x = 
    if !Magic.reorder_insns then 
      linearize(anticipate_friends use_same_vars x) 
    else 
      x
  in

  (* delay stores to the real and imaginary parts of the same number *)
  let x = 
    if !Magic.reorder_stores then 
      linearize(delay_friends store_to_same_class x) 
    else
      x
  in

  (* move loads of the real and imaginary parts of the same number *)
  let x = 
    if !Magic.reorder_loads then 
      linearize(anticipate_friends loads_from_same_class x) 
    else 
      x
  in

  let x = pair_stores list_of_pairable_stores x in
  let x = analyze [] x in
  let res = rewrite_declarations true x in
  let () = Util.info "end annotate" in
  res

let rec dump print (Annotate (_, _, _, _, code)) =
  dump_code print code
and dump_code print = function
  | ADone -> ()
  | AInstr x -> print ((assignment_to_string x) ^ "\n")
  | ASeq (a, b) -> dump print a; dump print b
