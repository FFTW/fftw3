(*
 * Copyright (c) 1997-1999 Massachusetts Institute of Technology
 * Copyright (c) 2003, 2006 Matteo Frigo
 * Copyright (c) 2003, 2006 Massachusetts Institute of Technology
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
(* $Id: expr.mli,v 1.13 2006-02-12 23:34:12 athena Exp $ *)

type transcendent = I | MULTI_A | MULTI_B

type expr =
  | Num of Number.number
  | NaN of transcendent
  | Plus of expr list
  | Times of expr * expr
  | CTimes of expr * expr
  | CTimesJ of expr * expr
  | Uminus of expr
  | Load of Variable.variable
  | Store of Variable.variable * expr

type assignment = Assign of Variable.variable * expr

val hash_float : float -> int
val hash : expr -> int
val to_string : expr -> string
val assignment_to_string : assignment -> string
val transcendent_to_float : transcendent -> float
val string_of_transcendent : transcendent -> string

val find_vars : expr -> Variable.variable list
val is_constant : expr -> bool
val is_known_constant : expr -> bool

val dump : (string -> unit) -> assignment list -> unit

val expr_to_constants : expr -> Number.number list
val unique_constants : Number.number list -> Number.number list
