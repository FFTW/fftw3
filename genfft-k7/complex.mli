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

(* $Id: complex.mli,v 1.5 2006-01-05 03:04:27 stevenj Exp $ *)

type expr
val make : (Expr.expr * Expr.expr) -> expr
val one : expr
val zero : expr
val i : expr
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
val wreflect : expr -> expr -> expr -> expr
type variable
val load_var : variable -> expr
val load_real : variable -> Expr.expr
val store_var : variable -> expr -> Expr.expr list
val store_real : variable -> expr -> Expr.expr list
val store_imag : variable -> expr -> Expr.expr list
val access_input : int -> variable
val access_output : int -> variable
val access_twiddle : int -> variable

val (@*) : expr -> expr -> expr
val (@+) : expr -> expr -> expr
val (@-) : expr -> expr -> expr

(* a signal is a map from integers to expressions *)
type signal = int -> expr
val infinite : int -> signal -> signal
