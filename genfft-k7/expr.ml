(*
 * Copyright (c) 1997-1999 Massachusetts Institute of Technology
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

(* $Id: expr.ml,v 1.2 2002-06-15 17:51:39 athena Exp $ *)

(* Here, we define the data type encapsulating a symbolic arithmetic
   expression, and provide some routines for manipulating it.  (See
   also simplify.ml for functions to do symbolic simplifications.) *)

type expr =
  | Num of Number.number
  | Var of Variable.variable
  | Plus of expr list
  | Times of expr * expr
  | Uminus of expr
  | Integer of int

type assignment = Assign of Variable.variable * expr

(* find all variables within a given expression. *)
let find_vars =
  let rec find_vars' xs = function
    | Var x      -> x::xs
    | Plus ps    -> List.fold_left find_vars' xs ps
    | Times(a,b) -> find_vars' (find_vars' xs b) a
    | Uminus a   -> find_vars' xs a
    | Num _	 -> xs 
    | Integer _	 -> xs
  in find_vars' []

(* debugging stuff *)
let rec foldr_string_concat l = 
  match l with
    [] -> ""
  | [a] -> a
  | a :: b -> a ^ " " ^ (foldr_string_concat b)

let rec expr_to_string = function
  | Var v -> Variable.unparse v
  | Num n -> string_of_float (Number.to_float n)
  | Plus x -> "(+ " ^ (foldr_string_concat (List.map expr_to_string x)) ^ ")"
  | Times (a, b) -> "(* " ^ (expr_to_string a) ^ " " ^ (expr_to_string b) ^ ")"
  | Uminus a -> "(- " ^ (expr_to_string a) ^ ")"
  | Integer x -> string_of_int x

and assignment_to_string = function
  | Assign (v, a) -> "(:= " ^ (Variable.unparse v) ^ " " ^
      (expr_to_string a) ^ ")"
