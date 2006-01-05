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
(* $Id: number.mli,v 1.4 2006-01-05 03:04:27 stevenj Exp $ *)

type number

val equal : number -> number -> bool
val of_int : int -> number
val zero : number
val one : number
val two : number
val mone : number
val is_zero : number -> bool
val is_one : number -> bool
val is_mone : number -> bool
val is_two : number -> bool
val mul : number -> number -> number
val div : number -> number -> number
val add : number -> number -> number
val sub : number -> number -> number
val negative : number -> bool
val greater : number -> number -> bool
val negate : number -> number
val sqrt : number -> number

(* cexp n i = (cos (2 * pi * i / n), sin (2 * pi * i / n)) *)
val cexp : int -> int -> (number * number)

val to_konst : number -> string
val to_string : number -> string
val to_float : number -> float

