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

(* This file contains the instruction scheduler, which finds an
   efficient ordering for a given list of instructions.

   The scheduler analyzes the DAG (Directed, Acyclic Graph) formed by
   the instruction dependencies, and recursively partitions it.  The
   resulting schedule data structure expresses a "good" ordering
   and structure for the computation.

   The scheduler makes use of utilties in Dag and other packages to
   manipulate the Dag and the instruction list. *)

(* In the standard distribution of FFTW 2.1.3 there is a module called
   'Schedule'. This module ('Vschedule') only slightly differs from 
   Schedule. It has been adapted to deal with structures defined in 
   module VSimdBasics. *)

open VDag
open VSimdBasics

(*************************************************
 *               Dag scheduler
 *************************************************)

let to_assignment node = node.instruction

let return x = x
let has_color c n = (n.color = c)
let set_color c n = (n.color <- c)
let has_either_color c1 c2 n = (n.color = c1 || n.color = c2)

let infinity = 100000

let node_is_reachable node = (node.label < infinity)
let node_has_no_predecessors node = (Util.null node.predecessors)
let node_has_no_successors   node = (Util.null node.successors)

let cc dag inputs =
  begin
    assert (inputs <> []);
    VDag.for_all dag (fun node -> node.label <- infinity);
    bfs dag (List.hd inputs) 0;
    let (reachable,unreachable) = 
      List.partition node_is_reachable (VDag.to_list dag) 
    in (List.map to_assignment reachable, List.map to_assignment unreachable)
  end

let rec connected_components alist =
  let dag = makedag alist in
  let inputs = List.filter node_has_no_predecessors (VDag.to_list dag) in
  match cc dag inputs with
    | a, [] -> [a]
    | a, b  -> a :: connected_components b

let loads_twiddle node =
  node.predecessors = [] &&
    (match VSimdInstrOperandSet.elements node.input_variables with
       | [x] -> vsimdinstroperand_is_twiddle x
       | _   -> false)

let partition alist =
  let dag = makedag alist in
  let dag' = VDag.to_list dag in
  let inputs  = List.filter node_has_no_predecessors dag'
  and outputs = List.filter node_has_no_successors   dag'
  and special_inputs = List.filter loads_twiddle dag' in
  begin
    VDag.for_all dag (fun node -> node.color <- BLACK);

    List.iter (set_color RED) inputs;

    (* The special inputs are input that read a twiddle factor.  They
       can end up either in the blue or in the red part.  If a red
       node needs a special input, the special input becomes red.  If
       all successors of a special input are blue, it becomes blue.
       Outputs are always blue.

       As a consequence, however, the final partition might be
       composed only of blue nodes (which is incorrect).  In this case
       we manually reset all inputs (whether special or not) to be red. *)

    List.iter (set_color YELLOW) special_inputs;
    List.iter (set_color BLUE) outputs;

    let rec loopi donep = 
      match (List.filter
	       (fun node -> (has_color BLACK node) &&
		 List.for_all (has_either_color RED YELLOW) node.predecessors)
	       dag') with
	[] -> if (donep) then () else loopo true
      |	i -> 
	  begin
	    List.iter (fun node -> 
	      begin
      		set_color RED node;
		List.iter (set_color RED) node.predecessors;
	      end) i;
	    loopo false; 
	  end

    and loopo donep =
      match (List.filter
	       (fun node -> (has_either_color BLACK YELLOW node) &&
		 List.for_all (has_color BLUE) node.successors)
	       dag') with
	[] -> if (donep) then () else loopi true
      |	o ->
	  begin
	    List.iter (set_color BLUE) o; 
	    loopi false; 
	  end

    (* among the magic parameters, this is the most obscure *)
    in if !Magic.loopo then 
      loopo false
    else
      loopi false;

    (* fix the partition if it is incorrect *)
    if not (List.exists (has_color RED) dag') then 
	List.iter (set_color RED) inputs; 
    
    return
      ((List.map to_assignment (List.filter (has_color RED) dag')),
       (List.map to_assignment (List.filter (has_color BLUE) dag')))
  end

type vschedule = 
  | VDone
  | VInstr of vsimdinstr
  | VSeq of vschedule * vschedule
  | VPar of vschedule list

let schedule =
  let rec schedule_alist = function
    | []    -> VDone
    | [a]   -> VInstr a
    | alist -> 
	match connected_components alist with
	  | [a] -> schedule_connected a
	  | l   -> VPar (List.map schedule_alist l)

  and schedule_connected alist = 
    let (a,b) = partition alist in VSeq(schedule_alist a,schedule_alist b)

  in schedule_alist

