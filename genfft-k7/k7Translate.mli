(*
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

open VSimdBasics
open K7Basics


val fail_unparser : string -> 'a -> 'b

val fixedstride_complex_unparser :
  int ->
  vintreg ->
  int -> VFpBasics.vfpaccess option -> k7vaddr

val unitstride_complex_unparser :
  vintreg ->
  int -> VFpBasics.vfpaccess option -> k7vaddr

val strided_real_unparser :
  vintreg * vintreg ->
  int -> VFpBasics.vfpaccess option -> k7vaddr

val strided_realofcomplex_unparser_withoffset :
  vintreg * vintreg * int ->
  int -> VFpBasics.vfpaccess option -> k7vaddr

val strided_imagofcomplex_unparser_withoffset :
  vintreg * vintreg * int ->
  int -> VFpBasics.vfpaccess option -> k7vaddr

val strided_real_split2_unparser :
  vintreg * vintreg * int * vintreg ->
  int -> VFpBasics.vfpaccess option -> k7vaddr

val strided_dualreal_unparser :
  vintreg * vintreg ->
  vintreg * vintreg ->
  int -> VFpBasics.vfpaccess option -> k7vaddr

val strided_complex_unparser :
  vintreg * vintreg ->
  int -> VFpBasics.vfpaccess option -> k7vaddr

val strided_complex_split2_unparser :
  vintreg * vintreg * int * vintreg ->
  int -> VFpBasics.vfpaccess option -> k7vaddr

val strided_hc2hc_unparser_1 :
  vintreg * vintreg * vintreg * 'a ->
  int -> VFpBasics.vfpaccess option -> k7vaddr

val strided_hc2hc_unparser_2 :
  vintreg * vintreg * vintreg * int ->
  int -> VFpBasics.vfpaccess option -> k7vaddr

val make_asm_unparser :
  'a * ('b -> 'c -> k7vaddr) ->
  'a * ('b -> 'c -> k7vaddr) ->
  'a * ('b -> 'c -> k7vaddr) ->
  'c -> 'b -> Variable.array -> 'a * k7vaddr

val make_asm_unparser_notwiddle :
  'a list * ('b -> 'c -> k7vaddr) ->
  'a list * ('b -> 'c -> k7vaddr) ->
  'c -> 'b -> Variable.array -> 'a list * k7vaddr

val vsimdinstrsToK7vinstrs :
  (VFpBasics.vfpaccess option ->
   int -> Variable.array -> k7vinstr list * k7vaddr) ->
  k7operandsize * vsimdinstr list ->
  k7vinstr list
