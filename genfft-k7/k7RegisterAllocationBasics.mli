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


type initcodeinstr = 
  | AddIntOnDemandCode of VSimdBasics.vintreg * K7Basics.k7vinstr list
  | FixRegister of VSimdBasics.vintreg * K7Basics.k7rintreg
and riregfileentry =
    IFree
  | IHolds of VSimdBasics.vintreg
  | IFixed of VSimdBasics.vintreg
  | ITmpHoldsProduct of VSimdBasics.vintreg * int
  | IVarHoldsProduct of VSimdBasics.vintreg * VSimdBasics.vintreg * int
and rsregfileentry = SFree | SHolds of VSimdBasics.vsimdreg
and vsregfileentry =
    SFresh
  | SDead
  | SDying of K7Basics.k7rmmxreg
  | SIn of K7Basics.k7rmmxreg
  | SInAndOut of K7Basics.k7rmmxreg * int
  | SInConst of K7Basics.k7rmmxreg * string
  | SOut of int
  | SConst of string
  | SOnDemand of K7Basics.k7vinstr list
and viregfileentry =
    IFresh
  | IDead
  | IDying of K7Basics.k7rintreg
  | IIn of K7Basics.k7rintreg
  | IInAndOut of K7Basics.k7rintreg * int
  | IInConst of K7Basics.k7rintreg * string
  | IOut of int
  | IConst of string
  | IOnDemand of K7Basics.k7vinstr list

val vsregfileentryIsFresh : vsregfileentry -> bool

val livevsregfileentryToK7rmmxreg :
  vsregfileentry -> K7Basics.k7rmmxreg option

val vsregfileentryToK7memop : vsregfileentry -> K7Basics.k7memop option
val vsregfileentryToK7rmmxreg : vsregfileentry -> K7Basics.k7rmmxreg option

val liveviregfileentryToK7rintreg :
  viregfileentry -> K7Basics.k7rintreg option

val viregfileentryToK7rintreg : viregfileentry -> K7Basics.k7rintreg option

val vsregfileentryToK7rmmxreg' : vsregfileentry -> K7Basics.k7rmmxreg

val viregfileentryToK7rintreg' : viregfileentry -> K7Basics.k7rintreg

val viregWantsIn :
  viregfileentry VSimdBasics.VIntRegMap.t ->
  VSimdBasics.VIntRegMap.key -> bool
val vsregWantsIn :
  vsregfileentry VSimdBasics.VSimdRegMap.t ->
  VSimdBasics.VSimdRegMap.key -> bool

val touchSimdDstReg :
  VSimdBasics.VSimdRegMap.key ->
  vsregfileentry VSimdBasics.VSimdRegMap.t ->
  vsregfileentry VSimdBasics.VSimdRegMap.t
val touchIntDstReg :
  VSimdBasics.VIntRegMap.key ->
  viregfileentry VSimdBasics.VIntRegMap.t ->
  viregfileentry VSimdBasics.VIntRegMap.t

val vsregfileentryIsFresh : vsregfileentry -> bool
val viregfileentryIsFresh : viregfileentry -> bool

val riregfileentryToVireg : riregfileentry -> VSimdBasics.vintreg option

val riregfileentryIsFree : riregfileentry -> bool
