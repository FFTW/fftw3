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


type vfpcomplexarrayformat = SplitFormat | InterleavedFormat
and vfparraytype = RealArray | ComplexArray of vfpcomplexarrayformat
val vfparraytypeIsReal : vfparraytype -> bool
val vfparraytypeIsComplex : vfparraytype -> bool
type vfpreg = V_FPReg of int
and vfpsummand = V_FPPlus of vfpreg | V_FPMinus of vfpreg
val makeNewVfpreg : unit -> vfpreg
module VFPRegSet :
  sig
    type elt = vfpreg
    and t
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
module VFPRegMap :
  sig
    type key = vfpreg
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
val vfpregmap_find : VFPRegMap.key -> 'a VFPRegMap.t -> 'a option
val vfpregmap_findE : VFPRegMap.key -> 'a list VFPRegMap.t -> 'a list
val vfpregmap_addE :
  VFPRegMap.key -> 'a -> 'a list VFPRegMap.t -> 'a list VFPRegMap.t
type vfpaccess = V_FPRealOfComplex | V_FPImagOfComplex | V_FPReal
and vfpunaryop = V_FPMulConst of Number.number | V_FPId | V_FPNegate
and vfpbinop = V_FPAdd | V_FPSub | V_FPMul
and vfpinstr =
    V_FPLoad of vfpaccess * Variable.array * int * vfpreg
  | V_FPStore of vfpreg * vfpaccess * Variable.array * int
  | V_FPUnaryOp of vfpunaryop * vfpreg * vfpreg
  | V_FPBinOp of vfpbinop * vfpreg * vfpreg * vfpreg
  | V_FPAddL of vfpsummand list * vfpreg
val vfpbinopIsAddorsub : vfpbinop -> bool
val vfpunaryopToNegated : vfpunaryop -> vfpunaryop
val vfpsummandToVfpreg : vfpsummand -> vfpreg
val vfpsummandIsPositive : vfpsummand -> bool
val vfpsummandIsNegative : vfpsummand -> bool
val vfpsummandToNegated : vfpsummand -> vfpsummand
val vfpinstrToSrcregs : vfpinstr -> vfpreg list
val vfpinstrToDstreg : vfpinstr -> vfpreg option
val vfpinstrToVfpregs : vfpinstr -> vfpreg list
val vfpinstrIsLoad : vfpinstr -> bool
val vfpinstrIsStore : vfpinstr -> bool
val vfpinstrIsLoadOrStore : vfpinstr -> bool
val vfpinstrToAddsubcount : vfpinstr -> int
val vfpinstrIsBinMul : vfpinstr -> bool
val vfpinstrIsUnaryOp : vfpinstr -> bool
val addlistHelper2 :
  vfpreg -> vfpsummand * vfpsummand -> vfpinstr * vfpsummand
val addlistHelper3' :
  vfpreg ->
  vfpreg ->
  vfpsummand * vfpsummand * vfpsummand -> vfpinstr * vfpinstr * vfpsummand
val addlistHelper3 :
  vfpreg ->
  vfpsummand * vfpsummand * vfpsummand -> vfpinstr * vfpinstr * vfpsummand
