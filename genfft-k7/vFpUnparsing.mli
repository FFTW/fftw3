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


val vfpregToString : VFpBasics.vfpreg -> string
val vfpregsToString : VFpBasics.vfpreg list -> string
val vfpsummandToString : VFpBasics.vfpsummand -> string
val vfpsummandsToString : VFpBasics.vfpsummand list -> string
val vfpaccessToString : VFpBasics.vfpaccess -> string
val vfpunaryopToString : VFpBasics.vfpunaryop -> string
val vfpbinopToString : VFpBasics.vfpbinop -> string
val vfpinstrToString : VFpBasics.vfpinstr -> string
