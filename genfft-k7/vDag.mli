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


type color = RED | BLUE | BLACK | YELLOW

and dagnode = {
  assigned : VSimdBasics.vsimdinstroperand list;
  instruction : VSimdBasics.vsimdinstr;
  input_variables : VSimdBasics.VSimdInstrOperandSet.t;
  input_variables_raw : VSimdBasics.VSimdRawVarSet.t;
  mutable successors : dagnode list;
  mutable predecessors : dagnode list;
  mutable label : int;
  mutable color : color;
} 

and dag = Dag of dagnode list

val node_uses : VSimdBasics.VSimdInstrOperandSet.elt -> dagnode -> bool
val node_clobbers : dagnode -> VSimdBasics.vsimdinstroperand -> bool

val depends_on : dagnode -> dagnode -> bool

val makedag : VSimdBasics.vsimdinstr list -> dag

val map : (dagnode -> dagnode) -> dag -> dag

val for_all : dag -> (dagnode -> 'a) -> unit

val to_list : dag -> dagnode list

val find_node : (dagnode -> bool) -> dag -> dagnode option

val bfs : dag -> dagnode -> int -> unit
