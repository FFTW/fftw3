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


type array = Input | Output | Twiddle

val arrayToString : array -> string

type variable =
    Temporary of int
  | Named of string
  | RealArrayElem of (array * int)
  | ImagArrayElem of (array * int)

val make_temporary : unit -> variable

val is_temporary : variable -> bool
val is_real : variable -> bool
val is_imag : variable -> bool
val is_output : variable -> bool
val is_input : variable -> bool
val is_twiddle : variable -> bool
val is_locative : variable -> bool
val is_constant : variable -> bool

val same : 'a -> 'a -> bool
val hash : variable -> int

val similar : variable -> variable -> bool
val clobbers : variable -> variable -> bool

val real_imag : variable -> variable -> bool
val increasing_indices : variable -> variable -> bool

val access : array -> int -> variable * variable
val access_input : int -> variable * variable
val access_output : int -> variable * variable
val access_twiddle : int -> variable * variable
val make_named : string -> variable

val unparse : variable -> string
