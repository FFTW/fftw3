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


val k7memopToString : K7Basics.k7memop -> string
val k7operandsizeToMovstring : K7Basics.k7operandsize -> string
val k7intbinopToString : K7Basics.k7intbinop -> string
val k7intunaryopToString : K7Basics.k7intunaryop -> string
val k7intcpyunaryopToString : K7Basics.k7intcpyunaryop -> string
val k7simdcpyunaryopToString : K7Basics.k7simdcpyunaryop -> string
val k7simdcpyunaryopmemToString : K7Basics.k7simdcpyunaryop -> string
val k7simdunaryopToString : K7Basics.k7simdunaryop -> string
val k7simdbinopToString : K7Basics.k7simdbinop -> string
val k7branchconditionToString : K7Basics.k7branchcondition -> string
val k7mmxstackcellToString : int -> string
val k7simdunaryopToArgstring : K7Basics.k7simdunaryop -> string
val k7intunaryopToArgstrings : K7Basics.k7intunaryop -> string list
val k7intcpyunaryopToArgstrings : K7Basics.k7intcpyunaryop -> string list
val offsetToString : int -> string
val scalarToString : int -> string
val k7vaddrToString : K7Basics.k7vaddr -> string
val k7vbranchtargetToString : K7Basics.k7vbranchtarget -> string
val k7vinstrToArgstrings : K7Basics.k7vinstr -> string list
val k7vinstrToInstrstring : K7Basics.k7vinstr -> string
val k7vinstrToString : K7Basics.k7vinstr -> string
val k7rintregToRawString : K7Basics.k7rintreg -> string
val k7rmmxregToRawString : K7Basics.k7rmmxreg -> string
val k7rintregToString : K7Basics.k7rintreg -> string
val k7rmmxregToString : K7Basics.k7rmmxreg -> string
val k7raddrToString : K7Basics.k7raddr -> string
val k7rbranchtargetToString : K7Basics.k7rbranchtarget -> string
val k7rinstrToArgstrings : K7Basics.k7rinstr -> string list
val k7rinstrToInstrstring : K7Basics.k7rinstr -> string
val k7rinstrToString : K7Basics.k7rinstr -> string

val k7rinstrInitStackPointerAdjustment : int -> unit
val k7rinstrAdaptStackPointerAdjustment : K7Basics.k7rinstr -> unit
