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

(* $Id: complex.mli,v 1.1 2002-06-14 10:56:15 athena Exp $ *)

type expr
val one : expr
val zero : expr
val inverse_int : int -> expr
val times : expr -> expr -> expr
val uminus : expr -> expr
val swap_re_im : expr -> expr
val exp : int -> int -> expr
val plus : expr list -> expr
val real : expr -> expr
val imag : expr -> expr
val conj : expr -> expr
val sigma : int -> int -> (int -> expr) -> expr
val wsquare : expr -> expr
val wthree : expr -> expr -> expr -> expr
type variable
val load_var : variable -> expr
val store_var : variable -> expr -> Exprdag.node list
val store_real : variable -> expr -> Exprdag.node list
val store_imag : variable -> expr -> Exprdag.node list
val access_input : int -> variable
val access_output : int -> variable
val access_twiddle : int -> variable
