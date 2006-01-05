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
(* $Id: twiddle.mli,v 1.4 2006-01-05 03:04:27 stevenj Exp $ *)

val speclist : (string * Arg.spec * string) list

type twinstr

val twiddle_policy :
    unit ->
    (int -> int -> (int -> Complex.expr) -> (int -> Complex.expr) ->
      int -> Complex.expr) *(int -> int) * (int -> twinstr list)

val twinstr_to_c_string : twinstr list -> string
val twinstr_to_asm_string : twinstr list -> string
