(*
 * Copyright (c) 1997-1999 Massachusetts Institute of Technology
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


val invmod : int -> int -> int
val gcd : int -> int -> int
val lowest_terms : int -> int -> int * int
exception No_Generator
val find_generator : int -> int
exception Negative_Power
val pow_mod : int -> int -> int -> int
val forall : 'a -> ('b -> 'a -> 'a) -> int -> int -> (int -> 'b) -> 'a
val sum_list : int list -> int
val max_list : int list -> int
val min_list : int list -> int
val count : ('a -> bool) -> 'a list -> int
val remove : 'a -> 'a list -> 'a list
val cons : 'a -> 'a list -> 'a list
val null : 'a list -> bool
val ( @@ ) : ('a -> 'b) -> ('c -> 'a) -> 'c -> 'b
val forall_flat : int -> int -> (int -> 'a list) -> 'a list
val identity : 'a -> 'a
val find_elem : ('a -> bool) -> 'a list -> 'a option
val suchthat : int -> (int -> bool) -> int
val selectFirst : ('a -> bool) -> 'a list -> ('a * 'a list) option
val insertList : ('a -> 'a -> bool) -> 'a -> 'a list -> 'a list
val insert_list : ('a -> 'a -> int) -> 'a -> 'a list -> 'a list
val zip : 'a list -> 'a list * 'a list
val intertwine : 'a list -> 'a list -> 'a list
val ( @. ) : 'a list * 'b list -> 'a list * 'b list -> 'a list * 'b list
val listAssoc : 'a -> ('a * 'b) list -> 'b option
val identity : 'a -> 'a
val listToString : ('a -> string) -> string -> 'a list -> string
val stringlistToString : string -> string list -> string
val intToString : int -> string
val floatToString : float -> string
val same_length : 'a list -> 'b list -> bool
val optionIsSome : 'a option -> bool
val optionIsNone : 'a option -> bool
val optionToValue' : exn -> 'a option -> 'a
val optionToValue : 'a option -> 'a
val optionToList : 'a option -> 'a list
val optionToListAndConcat : 'a list -> 'a option -> 'a list
val option_to_boolvaluepair : 'a -> 'a option -> bool * 'a
val minimize : ('a -> 'b) -> 'a list -> 'a option
val list_removefirst : ('a -> bool) -> 'a list -> 'a list
val cons : 'a -> 'a list -> 'a list
val mapOption : ('a -> 'b) -> 'a option -> 'b option
val get1of3 : 'a * 'b * 'c -> 'a
val get2of3 : 'a * 'b * 'c -> 'b
val get3of3 : 'a * 'b * 'c -> 'c
val get1of4 : 'a * 'b * 'c * 'd -> 'a
val get2of4 : 'a * 'b * 'c * 'd -> 'b
val get3of4 : 'a * 'b * 'c * 'd -> 'c
val get4of4 : 'a * 'b * 'c * 'd -> 'd
val get1of5 : 'a * 'b * 'c * 'd * 'e -> 'a
val get2of5 : 'a * 'b * 'c * 'd * 'e -> 'b
val get3of5 : 'a * 'b * 'c * 'd * 'e -> 'c
val get4of5 : 'a * 'b * 'c * 'd * 'e -> 'd
val get5of5 : 'a * 'b * 'c * 'd * 'e -> 'e
val get1of6 : 'a * 'b * 'c * 'd * 'e * 'f -> 'a
val get2of6 : 'a * 'b * 'c * 'd * 'e * 'f -> 'b
val get3of6 : 'a * 'b * 'c * 'd * 'e * 'f -> 'c
val get4of6 : 'a * 'b * 'c * 'd * 'e * 'f -> 'd
val get5of6 : 'a * 'b * 'c * 'd * 'e * 'f -> 'e
val get6of6 : 'a * 'b * 'c * 'd * 'e * 'f -> 'f
val repl1of2 : 'a -> 'b * 'c -> 'a * 'c
val repl2of2 : 'a -> 'b * 'c -> 'b * 'a
val repl1of3 : 'a -> 'b * 'c * 'd -> 'a * 'c * 'd
val repl2of3 : 'a -> 'b * 'c * 'd -> 'b * 'a * 'd
val repl3of3 : 'a -> 'b * 'c * 'd -> 'b * 'c * 'a
val repl1of4 : 'a -> 'b * 'c * 'd * 'e -> 'a * 'c * 'd * 'e
val repl2of4 : 'a -> 'b * 'c * 'd * 'e -> 'b * 'a * 'd * 'e
val repl3of4 : 'a -> 'b * 'c * 'd * 'e -> 'b * 'c * 'a * 'e
val repl4of4 : 'a -> 'b * 'c * 'd * 'e -> 'b * 'c * 'd * 'a
val repl1of5 : 'a -> 'b * 'c * 'd * 'e * 'f -> 'a * 'c * 'd * 'e * 'f
val repl2of5 : 'a -> 'b * 'c * 'd * 'e * 'f -> 'b * 'a * 'd * 'e * 'f
val repl3of5 : 'a -> 'b * 'c * 'd * 'e * 'f -> 'b * 'c * 'a * 'e * 'f
val repl4of5 : 'a -> 'b * 'c * 'd * 'e * 'f -> 'b * 'c * 'd * 'a * 'f
val repl5of5 : 'a -> 'b * 'c * 'd * 'e * 'f -> 'b * 'c * 'd * 'e * 'a
val repl1of6 :
  'a -> 'b * 'c * 'd * 'e * 'f * 'g -> 'a * 'c * 'd * 'e * 'f * 'g
val repl2of6 :
  'a -> 'b * 'c * 'd * 'e * 'f * 'g -> 'b * 'a * 'd * 'e * 'f * 'g
val repl3of6 :
  'a -> 'b * 'c * 'd * 'e * 'f * 'g -> 'b * 'c * 'a * 'e * 'f * 'g
val repl4of6 :
  'a -> 'b * 'c * 'd * 'e * 'f * 'g -> 'b * 'c * 'd * 'a * 'f * 'g
val repl5of6 :
  'a -> 'b * 'c * 'd * 'e * 'f * 'g -> 'b * 'c * 'd * 'e * 'a * 'g
val repl6of6 :
  'a -> 'b * 'c * 'd * 'e * 'f * 'g -> 'b * 'c * 'd * 'e * 'f * 'a
val fixpoint : ('a -> bool * 'a) -> 'a -> 'a
val return : 'a -> 'a
val diff : 'a list -> 'a list -> 'a list
val addelem : 'a -> 'a list -> 'a list
val union : 'a list -> 'a list -> 'a list
val uniq : 'a list -> 'a list
val msb : int -> int
val lists_overlap : 'a list -> 'a list -> bool
val toNil : 'a -> 'b list
val toNone : 'a -> 'b option
val toZero : 'a -> int
val info : string -> unit
val debugOutputString : string -> unit
val list_last : 'a list -> 'a
val array : int -> (int -> 'a) -> int -> 'a
val iota : int -> int list
val interval : int -> int -> int list
