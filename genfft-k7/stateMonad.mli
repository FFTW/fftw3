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


val unitM : 'a -> 'b -> 'a * 'b

val ( >>= ) : ('a -> 'b * 'c) -> ('b -> 'c -> 'd * 'e) -> 'a -> 'd * 'e
val ( >> ) : ('a -> 'b * 'c) -> ('c -> 'd * 'e) -> 'a -> 'd * 'e

val mapM : ('a -> 'b -> 'c * 'b) -> 'a list -> 'b -> 'c list * 'b

val mapPairM : ('a -> 'b -> 'c * 'b) -> 'a * 'a -> 'b -> ('c * 'c) * 'b
val mapTripleM :
  ('a -> 'b -> 'c * 'b) -> 'a * 'a * 'a -> 'b -> ('c * 'c * 'c) * 'b

val iterM : ('a -> 'b -> 'c * 'b) -> 'a list -> 'b -> unit * 'b

val ignoreM : ('a -> 'b * 'c) -> 'a -> unit * 'c

val runM : ('a -> 'b -> 'c * 'd) -> 'a -> 'b -> 'c

val fetchStateM : 'a -> 'a * 'a
val storeStateM : 'a -> 'b -> unit * 'a

val consM : 'a -> 'a list -> 'b -> 'a list * 'b
