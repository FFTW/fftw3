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


type vintreg = V_IntReg of int
and vsimdreg = V_SimdReg of int

val next_vintreg : vintreg -> vintreg
val next_vsimdreg : vsimdreg -> vsimdreg

val makeNewVsimdreg : unit -> vsimdreg
val makeNewVintreg : unit -> vintreg
val makeNewVintreg2 : unit -> vintreg * vintreg
val makeNewVintreg3 : unit -> vintreg * vintreg * vintreg

module VIntRegMap :
  sig
    type key = vintreg
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

val vintregmap_find : VIntRegMap.key -> 'a VIntRegMap.t -> 'a option
val vintregmap_findE : VIntRegMap.key -> 'a list VIntRegMap.t -> 'a list
val vintregmap_addE :
  VIntRegMap.key -> 'a -> 'a list VIntRegMap.t -> 'a list VIntRegMap.t
val vintregmap_cutE :
  VIntRegMap.key -> 'a list VIntRegMap.t -> 'a list VIntRegMap.t

module VSimdRegSet :
  sig
    type elt = vsimdreg
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

module VSimdRegMap :
  sig
    type key = vsimdreg
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

val vsimdregmap_find : VSimdRegMap.key -> 'a VSimdRegMap.t -> 'a option
val vsimdregmap_findE : VSimdRegMap.key -> 'a list VSimdRegMap.t -> 'a list
val vsimdregmap_addE :
  VSimdRegMap.key -> 'a -> 'a list VSimdRegMap.t -> 'a list VSimdRegMap.t
val vsimdregmap_addE' :
  'a -> VSimdRegMap.key -> 'a list VSimdRegMap.t -> 'a list VSimdRegMap.t
val vsimdregmap_cutE :
  VSimdRegMap.key -> 'a list VSimdRegMap.t -> 'a list VSimdRegMap.t

type vsimdpos = V_Lo | V_Hi | V_LoHi
and realimag = RealPart | ImagPart
and vconst =
    V_NumberPairConst of float * float
  | V_NamedConst of string
  | V_ChsConst of vsimdpos

module VConstMap :
  sig
    type key = vconst
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

type vsimdinstroperand =
    V_SimdTmp of int
  | V_SimdDVar of VFpBasics.vfpaccess * Variable.array * int
  | V_SimdQVar of Variable.array * int
  | V_SimdNConst of string
  | V_SimdNumConst of float * float

module VSimdInstrOperandSet :
  sig
    type elt = vsimdinstroperand
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

type vsimdrawvar =
    V_SimdRawQVar of int
  | V_SimdRawDVar of VFpBasics.vfpaccess * int

module VSimdRawVarSet :
  sig
    type elt = vsimdrawvar
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

val vsimdinstroperand_is_temporary : vsimdinstroperand -> bool
val vsimdinstroperand_is_twiddle : vsimdinstroperand -> bool

type vsimdbinop =
    V_Add
  | V_Sub
  | V_SubR
  | V_Mul
  | V_PPAcc
  | V_NNAcc
  | V_NPAcc
  | V_UnpckLo
  | V_UnpckHi
and vsimdunaryop =
    V_Id
  | V_Swap
  | V_Chs of vsimdpos
  | V_MulConst of Number.number * Number.number

val eq_vsimdunaryop : vsimdunaryop -> vsimdunaryop -> bool

val vsimdbinopIsParallel : vsimdbinop -> bool

type vsimdinstr =
    V_SimdLoadQ of Variable.array * int * vsimdreg
  | V_SimdStoreQ of vsimdreg * Variable.array * int
  | V_SimdLoadD of VFpBasics.vfpaccess * Variable.array * int * vsimdpos *
      vsimdreg
  | V_SimdStoreD of vsimdpos * vsimdreg * VFpBasics.vfpaccess *
      Variable.array * int
  | V_SimdUnaryOp of vsimdunaryop * vsimdreg * vsimdreg
  | V_SimdBinOp of vsimdbinop * vsimdreg * vsimdreg * vsimdreg

val vsimdinstrToSrcregs : vsimdinstr -> vsimdreg list
val vsimdinstrToDstreg : vsimdinstr -> vsimdreg option
val vsimdinstrToReads : vsimdinstr -> vsimdreg list
val vsimdinstrToWrites : vsimdinstr -> vsimdreg list
val vsimdinstrToReadswritespair : vsimdinstr -> vsimdreg list * vsimdreg list
val vsimdposToSwapped : vsimdpos -> vsimdpos
val vsimdposToSrcoperand : vsimdpos -> vsimdinstroperand
val vsimdunaryopToSrcoperands : vsimdunaryop -> vsimdinstroperand list
val vsimdinstrToSrcoperands : vsimdinstr -> vsimdinstroperand list
val vsimdinstrToSrcrawvars : vsimdinstr -> vsimdrawvar list
val vsimdinstrToDstoperands : vsimdinstr -> vsimdinstroperand list
val vsimdinstrIsLoad : vsimdinstr -> bool
val vsimdinstrIsStore : vsimdinstr -> bool
val scalarvfpunaryopToVsimdunaryop :
  VFpBasics.vfpunaryop -> vsimdpos -> vsimdunaryop

val vfpbinopToVsimdbinop : VFpBasics.vfpbinop -> vsimdbinop
