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

open Variable
open Expr
open VFpBasics

val varinfo_notwiddle : (array * vfparraytype) list
val varinfo_twiddle   : (array * vfparraytype) list

val varinfo_real2hc   : (array * vfparraytype) list
val varinfo_hc2real   : (array * vfparraytype) list

val varinfo_hc2hc_zerothelements : (array * vfparraytype) list
val varinfo_hc2hc_finalelements  : (array * vfparraytype) list
val varinfo_hc2hc_middleelements : (array * vfparraytype) list

val varinfo_realeven : (array * vfparraytype) list
val varinfo_realodd  : (array * vfparraytype) list

val assignmentsToVfpinstrs : 
  (array * vfparraytype) list -> assignment list -> vfpinstr list
