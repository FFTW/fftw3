(*
 * Copyright (c) 1997-1999 Massachusetts Institute of Technology
 * Copyright (c) 2002 Matteo Frigo
 * Copyright (c) 2002 Steven G. Johnson
 * Copyright (c) 2002 Stefan Kral
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
(* $Id: annotate.ml,v 1.4 2002-06-21 01:22:41 athena Exp $ *)

(* Here, we take a schedule (produced by schedule.ml) ordering a
   sequence of instructions, and produce an annotated schedule.  The
   annotated schedule has the same ordering as the original schedule,
   but is additionally partitioned into nested blocks of temporary
   variables.  The partitioning is computed via a heuristic algorithm.

   The blocking allows the C code that we generate to consist of
   nested blocks that help communicate variable lifetimes to the
   compiler. *)

(* $Id: annotate.ml,v 1.4 2002-06-21 01:22:41 athena Exp $ *)
open Schedule
open Expr
open Variable
open List

type annotated_schedule = 
    Annotate of variable list * variable list * variable list * int * aschedule
and aschedule = 
    ADone
  | AInstr of assignment
  | ASeq of (annotated_schedule * annotated_schedule)

let addelem a set = if not (List.mem a set) then a :: set else set
let union l = 
  let f x = addelem x   (* let is source of polymorphism *)
  in List.fold_right f l

(* set difference a - b *)
let diff a b = List.filter (fun x -> not (List.mem x b)) a

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
  List.fold_right (fun a b -> if List.mem a b then b else a :: b) l []

let has_related x = List.exists (Variable.same_class x)

let rec overlap a b = Util.count (fun y -> has_related y b) a


(* skral starts ------------------------------------------------------------ *)
type ldst =
  | MLoad
  | MStore

type useinfo = 
  | MUse of ldst * Variable.variable * Expr.expr
  | MTranspose of Variable.variable * Expr.expr
  | MTwid of Variable.variable * Variable.variable

type useinfo2 = 
  | MUseReIm of ldst * Variable.variable * Expr.expr * Variable.variable * Expr.expr
  | MTransposes of (Variable.variable * Expr.expr) list
  | MTwid2 of Variable.variable * Variable.variable * Variable.variable * Variable.variable

(* for transposed store, we need to have all scalar stores to n adjacent
 * numbers in one block. (n=|simd_vector|, typically 2 or 4).
 *)

let rec annotatedsched_varpairs = function
  | Annotate (_,_,_,_,asch) -> asch_varpairs asch
and ainstr_ldvarpairs = function
  | Assign (v, Load v') 
    when !Simdmagic.collect_load && Variable.is_locative v' -> 
      [MUse(MLoad,v',Load v)]
  | Assign (v, Load v')
    when !Simdmagic.collect_twiddle && Variable.is_constant v' &&
	 (is_real v' || is_imag v') -> 
      [MTwid(v, v')]
  | _ -> []
and ainstr_stvarpairs = function
  | Assign (v, e) 
    when !Simdmagic.store_transpose && Variable.is_locative v ->
      [MTranspose(v,e)]
  | Assign (v, e) 
    when !Simdmagic.collect_store && Variable.is_locative v -> 
      [MUse(MStore,v,e)]
  | _ -> []
and asch_varpairs = function
  | ADone -> []
  | AInstr i -> (ainstr_ldvarpairs i) @ (ainstr_stvarpairs i)
  | ASeq (a,b) -> (annotatedsched_varpairs a) @ (annotatedsched_varpairs b)

let locativeToArea v = var_index v / !Simdmagic.vector_length 

let similarpairs a b = match (a,b) with
  | MTwid(d1,s1), MTwid(d2,s2) ->
      var_index s1 = var_index s2

  | MUse(t1,v1,e1), MUse(t2,v2,e2) -> 
      t1=t2 && (Variable.same_class v1 v2)
      (* does not work: 
		(is_real v1 && is_real v2 || is_imag v1 && is_imag v2)
	*)
  | MTranspose(v1,_),  MTranspose(v2,_) -> 
      locativeToArea v1 = locativeToArea v2 && 
      ((is_real v1 && is_real v2) || (is_imag v1 && is_imag v2)) 
  | _ -> false

let useinfoToTransposevar = function
  | MTranspose(x,e) -> (x,e)
  | _ -> failwith "useinfoToTransposevar"

let combineuseinfos = function
  | [MUse(t1,v1,e1); MUse(t2,v2,e2)] -> MUseReIm(t1,v1,e1,v2,e2)
  | [MTwid(d1,s1);   MTwid(d2,s2)]   -> MTwid2(d1,s1,d2,s2)
  | zs -> MTransposes (map useinfoToTransposevar zs)

(* use List.partition *)
let selectP pred =
  let rec selectP2 prefix = function
    | []    -> None
    | x::xs -> if pred x then Some(x,prefix@xs) else selectP2 (x::prefix) xs
  in selectP2 []

let rec collectpairs = function
  | [] -> []
  | x::xs -> 
      match List.partition (similarpairs x) xs with
        | ([], _)  -> failwith "collectpairs: internal error!"
	| (xs, zs) -> (combineuseinfos (x::xs))::(collectpairs zs)

let combineuseinfoToDeclvars = function
  | MTwid2(d1,s1,d2,s2)     -> 
      if !Simdmagic.collect_twiddle then [d1;d2] else []
  | MUseReIm(_,v1,e1,v2,e2) -> [v1;v2] @ (find_vars e1) @ (find_vars e2)
  | MTransposes vs 	    -> concat (map (fun (_,e) -> find_vars e) vs)

let collectedpairs_to_declvars = 
  map (fun d -> filter is_temporary (combineuseinfoToDeclvars d))

(* skral ends -------------------------------------------------------------- *)


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

let rec rewrite_declarations force_declarations var_decls
    (Annotate (_, _, declared, _, what)) =
  let m = !Magic.number_of_variables in

  (** skral: decide upon delayed declaration of variables *)
  let declare_it declared =	
    if (List.exists (fun xs -> 
	  (List.exists (fun x -> List.mem x declared) xs) && 
	   (not (List.for_all (fun x -> List.mem x declared) xs))) var_decls) then
	if force_declarations then
	  failwith("Annotate.rewrite_declarations: Internal Error!")
	else
	  (declared, [])
    else
        if (force_declarations or List.length declared >= m) then
          ([], declared)
        else
          (declared, [])

(*
  let declare_it declared =
    if (force_declarations or List.length declared >= m) then
      ([], declared)
    else
      (declared, [])
*)

  in match what with
    ADone -> Annotate ([], [], [], 0, what)
  | AInstr i -> 
      let (u, d) = declare_it declared
      in Annotate ([], u, d, 0, what)
  | ASeq (a, b) ->
      let ma = rewrite_declarations false var_decls a
      and mb = rewrite_declarations false var_decls b
      in let Annotate (_, ua, _, _, _) = ma
      and Annotate (_, ub, _, _, _) = mb
      in let (u, d) = declare_it (declared @ ua @ ub)
      in Annotate ([], u, d, 0, ASeq (ma, mb))

let annotate schedule =
  let m = !Magic.number_of_variables in

  let rec really_analyze live_at_end = function
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

  (* the pick_best machinery is currently unused. Too slow, and
     I don't know what `best' means *)
  and analyze l =
    let pick_best w = 
      minimize 
	(function Annotate (_, _, _, depth, _) -> depth)
	(List.map (really_analyze l) w)
    in function

      |	Seq (a, Done) -> analyze l a
      |	Seq (Done, a) -> analyze l a

       (* try to balance nested Par blocks *)
      |	Par [a] -> pick_best [a]
      |	Par l -> 
	  let n2 = (List.length l) / 2 in
	  let rec loop n a b =
	    if n = 0 then
	      (List.rev b, a)
	    else
	      match a with
		[] -> failwith "loop"
	      |	x :: y -> loop (n - 1) y (x :: b)
	  in let (a, b) = loop n2 (reorder l) []
	  in pick_best [Seq (Par a, Par b)]

      | x -> pick_best [x]

  in
  let intermediate_result = analyze [] schedule in
  let vardeclinfo = collectpairs (annotatedsched_varpairs intermediate_result)
  in 
  (vardeclinfo, (rewrite_declarations 
		   true 
		   (collectedpairs_to_declvars vardeclinfo) 
		   intermediate_result))



