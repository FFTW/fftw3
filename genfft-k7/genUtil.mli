(*
 * Copyright (c) 2001 Stefan Kral
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

open VFpBasics
open VSimdBasics
open K7Basics

val vect_optimize :
  (Variable.array * vfparraytype) list ->
  int -> Exprdag.dag -> k7operandsize * vsimdinstr list

val get2ndhalfcode :
  vintreg -> vintreg -> vintreg -> int -> k7vinstr list

val loadfnargs :
  (k7memop * vintreg) list -> (vintreg * k7vinstr list) list

val compileToAsm :
  int * Fft.direction * CodeletMisc.codelet_type *
  K7RegisterAllocationBasics.initcodeinstr list * k7vinstr list -> unit

val compileToAsm_with_fixedstride :
  (int * Fft.direction * CodeletMisc.codelet_type *
   K7RegisterAllocationBasics.initcodeinstr list * k7vinstr list) ->
  (int * int) -> unit
