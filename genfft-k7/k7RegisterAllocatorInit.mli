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

open VSimdBasics
open K7Basics
open K7RegisterAllocationBasics


val prepareRegAlloc :
  initcodeinstr list -> k7vinstr list ->
    ((k7rmmxreg * rsregfileentry) list * (k7rintreg * riregfileentry) list) *
    (vsregfileentry VSimdRegMap.t * viregfileentry VIntRegMap.t) *
    (int list VSimdRegMap.t * int list VIntRegMap.t) *
    k7vinstr list
