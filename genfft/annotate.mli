(*
 * Copyright (c) 1997-1999 Massachusetts Institute of Technology
 * Copyright (c) 2000 Matteo Frigo
 * Copyright (c) 2000 Steven G. Johnson
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
(* $Id: annotate.mli,v 1.2 2002-06-20 19:04:37 athena Exp $ *)

open Variable
open Expr

type annotated_schedule = 
    Annotate of variable list * variable list * variable list *
	int * aschedule
and aschedule = 
    ADone
  | AInstr of assignment
  | ASeq of (annotated_schedule * annotated_schedule)

type ldst = 
  | MLoad 
  | MStore

type useinfo = 
  | MUse of ldst * Variable.variable * Expr.expr
  | MTranspose of Variable.variable * Expr.expr
  | MTwid of Variable.variable * Variable.variable

type useinfo2 = 
  | MUseReIm of ldst * Variable.variable * Expr.expr * Variable.variable * Expr.expr
  | MTransposes of (Variable.variable * Expr.expr) list
  | MTwid2 of Variable.variable * Variable.variable * Variable.variable * Variable.variable

val annotate : Schedule.schedule -> useinfo2 list * annotated_schedule



