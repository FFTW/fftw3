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


val cvsid : string

val ( @* ) : Complex.expr -> Complex.expr -> Complex.expr
val ( @+ ) : Complex.expr -> Complex.expr -> Complex.expr
val ( @- ) : Complex.expr -> Complex.expr -> Complex.expr

val choose_factor : int -> int
val freeze : int -> (int -> 'a) -> int -> 'a

val fftgen_prime : int -> (int -> Complex.expr) -> int -> int -> Complex.expr
val fftgen_rader : int -> (int -> Complex.expr) -> int -> int -> Complex.expr

val fftgen :
  int ->
  Symmetry.symmetry -> (int -> Complex.expr) -> int -> int -> Complex.expr

type direction = FORWARD | BACKWARD

val sign_of_dir : direction -> int
val conj_of_dir : direction -> Complex.expr -> Complex.expr
val dagify : int -> Symmetry.symmetry -> (int -> Complex.expr) -> Exprdag.dag

val no_twiddle_gen_expr :
  int -> Symmetry.symmetry -> direction -> Exprdag.dag

val twiddle_dit_gen_expr :
  int -> Symmetry.symmetry -> Symmetry.symmetry -> direction -> Exprdag.dag

val twiddle_dif_gen_expr :
  int -> Symmetry.symmetry -> Symmetry.symmetry -> direction -> Exprdag.dag
