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

(* Here, we have functions to transform a sequence of assignments
   (variable = expression) into a DAG (a directed, acyclic graph).
   The nodes of the DAG are the assignments, and the edges indicate
   dependencies.  (The DAG is analyzed in the scheduler to find an
   efficient ordering of the assignments.)

   This file also contains utilities to manipulate the DAG in various
   ways. *)

(* In the standard distribution of FFTW 2.1.3 there is a module called Dag.
   This module (Vdag) is an adaptation of that module (Dag) that operates
   on vsimdinstructions instead of expressions and assignments. *)

open Variable
open Number
open VSimdBasics
open List

(********************************************
 *  Dag structure
 ********************************************)
type color = RED | BLUE | BLACK | YELLOW

type dagnode = 
    { assigned		   : vsimdinstroperand list;
      instruction  	   : vsimdinstr;
      input_variables	   : VSimdInstrOperandSet.t;
      input_variables_raw  : VSimdRawVarSet.t;
      mutable successors   : dagnode list;
      mutable predecessors : dagnode list;
      mutable label 	   : int;
      mutable color	   : color }

type dag = Dag of dagnode list

(* true if node uses v *)
let node_uses v node =
  VSimdInstrOperandSet.mem v node.input_variables

(* true if assignment of v clobbers any input of node *)
let node_clobbers node = function
  | V_SimdQVar(Output,k) ->
      VSimdRawVarSet.mem (V_SimdRawQVar k) node.input_variables_raw
  | V_SimdDVar(p,Output,k) ->
      VSimdRawVarSet.mem (V_SimdRawDVar(p,k)) node.input_variables_raw
  | _ -> false

(* true if nodeb depends on nodea *)
let depends_on nodea nodeb = 
  exists (fun i -> node_uses i nodeb) nodea.assigned || 
  exists (fun i -> node_clobbers nodea i) nodeb.assigned

(* transform an assignment list into a dag *)
let makedag alist =
  let dag = List.map
      (fun instr ->
	{ assigned            = vsimdinstrToDstoperands instr;
	  instruction         = instr;
	  input_variables     = List.fold_right VSimdInstrOperandSet.add 
	  				        (vsimdinstrToSrcoperands instr) 
					        VSimdInstrOperandSet.empty;
	  input_variables_raw = List.fold_right VSimdRawVarSet.add 
						(vsimdinstrToSrcrawvars instr)
						VSimdRawVarSet.empty;
	  successors          = [];
	  predecessors        = [];
	  label 	      = 0;
	  color 	      = BLACK })
      alist
  in begin
    List.iter (fun i ->
	List.iter (fun j ->
	  if depends_on i j then begin
	    i.successors <- j :: i.successors;
	    j.predecessors <- i :: j.predecessors;
	  end) dag) dag;
    Dag dag;
  end

let map f (Dag dag) = Dag (List.map f dag)
let for_all (Dag dag) f = 
  (* type system loophole *)
  let make_unit _ = () in
  make_unit (List.map f dag)
let to_list (Dag dag) = dag
let find_node f (Dag dag) = Util.find_elem f dag

(* breadth-first search *)
let rec bfs (Dag dag) node init_label =
  let _ =  node.label <- init_label in
  let rec loop = function
      [] -> ()
    | node :: rest ->
	let neighbors = node.predecessors @ node.successors in
	let m = Util.min_list (List.map (fun node -> node.label) neighbors) in
	if (node.label > m + 1) then begin
	  node.label <- m + 1;
	  loop (rest @ neighbors);
	end else
	  loop rest
  in let neighbors = node.predecessors @ node.successors in
  loop neighbors

