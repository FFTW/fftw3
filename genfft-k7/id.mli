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


module Id :
  sig
    type t
    val makeNew : unit -> t
    val equal : t -> t -> bool
    val compare : t -> t -> int
    val toString : t -> string
  end
module IdSet :
  sig
    type elt = Id.t
    and t = Set.Make(Id).t
    val empty : t
    val is_empty : t -> bool
    val mem : elt -> t -> bool
    val add : elt -> t -> t
    val singleton : elt -> t
    val remove : elt -> t -> t
    val union : t -> t -> t
    val inter : t -> t -> t
    val diff : t -> t -> t
    val compare : t -> t -> int
    val equal : t -> t -> bool
    val subset : t -> t -> bool
    val iter : (elt -> unit) -> t -> unit
    val fold : (elt -> 'a -> 'a) -> t -> 'a -> 'a
    val for_all : (elt -> bool) -> t -> bool
    val exists : (elt -> bool) -> t -> bool
    val filter : (elt -> bool) -> t -> t
    val partition : (elt -> bool) -> t -> t * t
    val cardinal : t -> int
    val elements : t -> elt list
    val min_elt : t -> elt
    val max_elt : t -> elt
    val choose : t -> elt
  end
module IdMap :
  sig
    type key = Id.t
    and 'a t = 'a Map.Make(Id).t
    val empty : 'a t
    val add : key -> 'a -> 'a t -> 'a t
    val find : key -> 'a t -> 'a
    val remove : key -> 'a t -> 'a t
    val mem : key -> 'a t -> bool
    val iter : (key -> 'a -> unit) -> 'a t -> unit
    val map : ('a -> 'b) -> 'a t -> 'b t
    val mapi : (key -> 'a -> 'b) -> 'a t -> 'b t
    val fold : (key -> 'a -> 'b -> 'b) -> 'a t -> 'b -> 'b
  end
val idmapToXs :
  ('a -> IdMap.key -> 'b -> 'b) ->
  ('c -> 'a list) -> 'c IdMap.t -> 'b -> 'b
val listToIdmap : 'a list -> 'a IdMap.t
val idmap_exists : IdMap.key -> 'a IdMap.t -> bool
val idmap_inc : IdMap.key -> int IdMap.t -> int IdMap.t
val idmap_findE : IdMap.key -> 'a list IdMap.t -> 'a list
val idmap_addE : IdMap.key -> 'a -> 'a list IdMap.t -> 'a list IdMap.t
val idmap_addE' : 'a -> IdMap.key -> 'a list IdMap.t -> 'a list IdMap.t
