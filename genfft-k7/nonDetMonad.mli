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


val identityM : 'a -> 'b -> ('a * 'b) option
val unitM : 'a -> 'b -> ('a -> 'b -> 'c) -> 'c
val ( >>= ) :
  ('a -> ('b -> 'c -> 'd) -> 'e) -> ('b -> 'c -> 'f -> 'd) -> 'a -> 'f -> 'e
val ( >> ) :
  ('a -> ('b -> 'c -> 'd) -> 'e) -> ('c -> 'f -> 'd) -> 'a -> 'f -> 'e
val fetchStateM : 'a -> ('a -> 'a -> 'b) -> 'b
val storeStateM : 'a -> 'b -> (unit -> 'a -> 'c) -> 'c
val catch1M :
  'a ->
  (unit -> 'b -> (unit -> 'b -> 'c) -> 'c) -> 'b -> (unit -> 'b -> 'c) -> 'c
val ( ||| ) :
  (unit -> 'a -> 'b -> ('c * 'd) option) ->
  (unit -> 'a -> 'b -> ('c * 'd) option) -> 'a -> 'b -> ('c * 'd) option
val failM : 'a -> 'b -> 'c option
val runM :
  ('a -> 'b -> ('c -> 'd -> ('c * 'd) option) -> ('e * 'f) option) ->
  'a -> 'b -> 'e option
val runP :
  ('a -> 'b -> ('c -> 'd -> ('c * 'd) option) -> ('e * 'f) option) ->
  'a -> 'b -> bool
val posAssertM : bool -> 'a -> (unit -> 'a -> 'b option) -> 'b option
val negAssertM : bool -> 'a -> (unit -> 'a -> 'b option) -> 'b option



val mapPairM :
  ('a -> 'b -> ('c -> 'b -> 'd) -> 'd) ->
  'a * 'a -> 'b -> ('c * 'c -> 'b -> 'd) -> 'd
val mapTripleM :
  ('a -> 'b -> ('c -> 'b -> 'd) -> 'd) ->
  'a * 'a * 'a -> 'b -> ('c * 'c * 'c -> 'b -> 'd) -> 'd
val consM : 'a -> 'a list -> 'b -> ('a list -> 'b -> 'c) -> 'c
val mapM :
  ('a -> 'b -> ('c -> 'b -> 'd) -> 'd) ->
  'a list -> 'b -> ('c list -> 'b -> 'd) -> 'd
val optionToValueM : 'a option -> 'b -> ('a -> 'b -> 'c option) -> 'c option
val iterM :
  ('a -> 'b -> ('c -> 'b -> 'd) -> 'd) ->
  'a list -> 'b -> (unit -> 'b -> 'd) -> 'd
val iterirevM :
  (int -> 'a -> 'b -> (unit -> 'b -> 'c) -> 'c) ->
  int -> 'a list -> 'b -> (unit -> 'b -> 'c) -> 'c
val memberM' :
  'a -> 'a list -> 'b -> ('a -> 'b -> ('c * 'd) option) -> ('c * 'd) option
val memberM :
  'a list -> 'b -> ('a -> 'b -> ('c * 'd) option) -> ('c * 'd) option
val selectM' :
  'a ->
  'a list ->
  'b -> ('a * 'a list -> 'b -> ('c * 'd) option) -> ('c * 'd) option
val selectM :
  'a list ->
  'b -> ('a * 'a list -> 'b -> ('c * 'd) option) -> ('c * 'd) option
val selectFirstM :
  ('a -> bool) ->
  'a list -> 'b -> ('a * 'a list -> 'b -> 'c option) -> 'c option
val deleteFirstM :
  ('a -> bool) -> 'a list -> 'b -> ('a list -> 'b -> 'c option) -> 'c option
val permutationM :
  'a list -> 'b -> ('a list -> 'b -> ('c * 'd) option) -> ('c * 'd) option
val forallM :
  ('a -> 'b -> ('c -> 'b -> 'd) -> 'd) ->
  'a list -> 'b -> (unit -> 'b -> 'd) -> 'd
val existsM :
  ('a -> 'b -> 'c -> ('d * 'e) option) ->
  'a list -> 'b -> 'c -> ('d * 'e) option
val betweenM' :
  int ->
  int -> int -> 'a -> (int -> 'a -> ('b * 'c) option) -> ('b * 'c) option
val betweenM :
  int -> int -> 'a -> (int -> 'a -> ('b * 'c) option) -> ('b * 'c) option
val disjM :
  (unit -> 'a -> 'b -> ('c * 'd) option) list -> 'a -> 'b -> ('c * 'd) option
