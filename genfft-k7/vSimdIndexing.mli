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


type vsimdinstrcategory =
    VSIC_Load
  | VSIC_Store
  | VSIC_UnaryMulConst
  | VSIC_UnaryCopy
  | VSIC_UnarySwap
  | VSIC_UnaryChsLo
  | VSIC_UnaryChsHi
  | VSIC_UnaryChsLoHi
  | VSIC_BinAdd
  | VSIC_BinSub
  | VSIC_BinMul
  | VSIC_BinPPAcc
  | VSIC_BinNNAcc
  | VSIC_BinNPAcc
  | VSIC_BinUnpckLo
  | VSIC_BinUnpckHi

module VSICMap :
  sig
    type key = vsimdinstrcategory
    and (+'a) t
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

val vsicmap_findE : VSICMap.key -> 'a list VSICMap.t -> 'a list
val vsicmap_addE :
  VSICMap.key -> 'a -> 'a list VSICMap.t -> 'a list VSICMap.t

val vsimdunaryopcategories_chs : vsimdinstrcategory list
val vsimdunaryopcategories_nocopy : vsimdinstrcategory list
val vsimdunaryopcategories_all : vsimdinstrcategory list
val vsimdbinopcategories_par : vsimdinstrcategory list
val vsimdbinopcategories_all : vsimdinstrcategory list
val vsimdallcategories : vsimdinstrcategory list
val vsimdunaryopToCategory : VSimdBasics.vsimdunaryop -> vsimdinstrcategory
val vsimdbinopToCategory : VSimdBasics.vsimdbinop -> vsimdinstrcategory
val vsimdinstrToCategory : VSimdBasics.vsimdinstr -> vsimdinstrcategory
val vsimdinstrToCategories :
  VSimdBasics.vsimdinstr -> vsimdinstrcategory list
