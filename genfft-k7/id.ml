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

open List

module Id : sig
  type t

  val makeNew  : unit -> t
  val equal    : t -> t -> bool
  val compare  : t -> t -> int
  val toString : t -> string
end = struct
  type t = ID of int

  let makeNew = 
    let currentId = ref 0 in
      fun () -> 
        incr currentId; 
        ID !currentId

  let toString (ID i) = Printf.sprintf "id(%d)" i
  let equal = (=)
  let compare = compare
end

module IdSet = Set.Make(Id)
module IdMap = Map.Make(Id)

let idmapToXs keymap_addE valueToXs =
  IdMap.fold
    (fun id value -> 
       fold_right (fun key -> keymap_addE key id) (valueToXs value))

let listToIdmap xs = 
  fold_right (fun x -> IdMap.add (Id.makeNew ()) x) xs IdMap.empty

let idmap_exists k m = try IdMap.find k m; true with Not_found -> false
let idmap_inc key map = IdMap.add key (IdMap.find key map + 1) map

(* add to list of existing entries *)
let idmap_findE k m = try IdMap.find k m with Not_found -> []
let idmap_addE k v m = IdMap.add k (v::idmap_findE k m) m
let idmap_addE' value key = idmap_addE key value

