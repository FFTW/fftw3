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


type k7rflag = K7RFlag_Zero
and k7rop =
    K7ROp_MMXReg of K7Basics.k7rmmxreg
  | K7ROp_IntReg of K7Basics.k7rintreg
  | K7ROp_MemOp of K7Basics.k7memop
  | K7ROp_IP
  | K7ROp_Flag of k7rflag
  | K7ROp_MMXState
  | K7ROp_MemoryState
  | K7ROp_Stack

val zeroflag : k7rop
val mmxstackcell : int -> k7rop

module ResMap :
  sig
    type key = k7rop
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

val resmap_findE : ResMap.key -> 'a list ResMap.t -> 'a list
val resmap_addE : ResMap.key -> 'a -> 'a list ResMap.t -> 'a list ResMap.t
val resmap_addE' : 'a -> ResMap.key -> 'a list ResMap.t -> 'a list ResMap.t
val resmap_find : ResMap.key -> 'a ResMap.t -> 'a option

val raddrToRops : K7Basics.k7raddr -> k7rop list
val intuopToSrcrops : k7rop -> K7Basics.k7intunaryop -> k7rop list
val intcpyuopToSrcrops : K7Basics.k7intcpyunaryop -> 'a list
val branchconditionToSrcrops : K7Basics.k7branchcondition -> k7rop list
val simduopToSrcrops : 'a -> K7Basics.k7simdunaryop -> 'a list
val simdcpyuopToSrcrops : K7Basics.k7simdcpyunaryop -> 'a list
val rinstrToSrcrops' : K7Basics.k7rinstr -> k7rop list
val intuopToDstrops : k7rop -> K7Basics.k7intunaryop -> (k7rop * int) list
val intcpyuopToDstrops : 'a -> K7Basics.k7intcpyunaryop -> ('a * int) list

val simduopToLatency : K7Basics.k7simdunaryop -> int
val simdbopToLatency : K7Basics.k7simdbinop -> int

val k7rinstrToSrck7rops : K7Basics.k7rinstr -> k7rop list
val k7rinstrToDstk7rops : K7Basics.k7rinstr -> (k7rop * int) list

val k7rinstrToMaxlatency : K7Basics.k7rinstr -> int

val k7rinstrCannotRollOverK7rinstr :
  K7Basics.k7rinstr -> K7Basics.k7rinstr -> bool
