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
(* $Id: genutil.ml,v 1.1.1.1 2002-06-02 18:42:29 athena Exp $ *)

(* utilities common to all generators *)
open Util

let unique_array n = array n (fun _ -> Unique.make ())
let unique_array_c n = 
  array n (fun _ -> 
    (Unique.make (), Unique.make ()))

let unique_v_array_c veclen n = 
  array veclen (fun _ ->
    unique_array_c n)

let locative_array_c n rarr iarr loc = 
  array n (fun i -> 
    let klass = Unique.make () in
    let (rloc, iloc) = loc i in
    (Variable.make_locative rloc klass (rarr i),
     Variable.make_locative iloc klass (iarr i)))

let locative_v_array_c veclen n rarr iarr loc = 
  array veclen (fun v ->
    array n (fun i -> 
      let klass = Unique.make () in
      let (rloc, iloc) = loc v i in
      (Variable.make_locative rloc klass (rarr v i),
       Variable.make_locative iloc klass (iarr v i))))

let temporary_array n = 
  array n (fun i -> Variable.make_temporary ())

let temporary_array_c n = 
  let tmpr = temporary_array n
  and tmpi = temporary_array n
  in 
  array n (fun i -> (tmpr i, tmpi i))

let temporary_v_array_c veclen n =
  array veclen (fun v -> temporary_array_c n)

let load_constant_array_r n arr = 
  array n (fun i -> 
    let klass = Unique.make () in
    let aref = C.array_subscript arr (C.SInteger 1) i in
    Expr.Load (Variable.make_constant klass aref))

let constant_array_c n arr = 
  array n (fun i -> 
    let klass = Unique.make () in
    let aref = C.array_subscript arr (C.SInteger 1) i in
    (Variable.make_constant klass (C.real_of aref),
     Variable.make_constant klass (C.imag_of aref)))

let constant_v_array_c veclen n arr vstride =
  array veclen (fun v -> 
    array n (fun i -> 
      let klass = Unique.make () in
      let aref = C.varray_subscript arr vstride (C.SInteger 1) v i in
      (Variable.make_constant klass (C.real_of aref),
       Variable.make_constant klass (C.imag_of aref))))

let temporary_array_c n = 
  let tmpr = temporary_array n
  and tmpi = temporary_array n
  in 
  array n (fun i -> (tmpr i, tmpi i))

let load_c (vr, vi) = Complex.make (Expr.Load vr, Expr.Load vi)
let load_r (vr, vi) = Complex.make (Expr.Load vr, Expr.Num (Number.zero))

let load_array_c n var = array n (fun i -> load_c (var i))
let load_array_r n var = array n (fun i -> load_r (var i))

let load_v_array_c veclen n var =
  array veclen (fun v -> load_array_c n (var v))

let store_c (vr, vi) x = [Complex.store_real vr x; Complex.store_imag vi x]
let store_r (vr, vi) x = Complex.store_real vr x
let store_i (vr, vi) x = Complex.store_imag vi x

let assign_array_c n dst src =
  List.flatten
    (rmap (iota n)
       (fun i ->
	 let (ar, ai) = Complex.assign (dst i) (src i)
	 in [ar; ai]))
let assign_v_array_c veclen n dst src =
  List.flatten
    (rmap (iota veclen)
       (fun v ->
	 assign_array_c n (dst v) (src v)))

let vassign_v_array_c veclen n dst src =
  List.flatten
    (rmap (iota n) (fun i ->
      List.flatten
	(rmap (iota veclen)
	   (fun v ->
	     let (ar, ai) = Complex.assign (dst v i) (src v i)
	     in [ar; ai]))))

let store_array_r n dst src =
  rmap (iota n)
    (fun i -> store_r (dst i) (src i))

let store_array_c n dst src =
  List.flatten
    (rmap (iota n)
       (fun i -> store_c (dst i) (src i)))

let store_array_hc n dst src =
  rmap (iota n)
    (fun i -> 
      if (i <= n - i) then
	store_r (dst i) (Complex.real (src i))
      else
	store_r (dst i) (Complex.imag (src (n - i))))

let store_v_array_c veclen n dst src =
  List.flatten
    (rmap (iota veclen)
       (fun v ->
	 store_array_c n (dst v) (src v)))


let elementwise f n a = array n (fun i -> f (a i))
let conj_array_c = elementwise Complex.conj
let real_array_c = elementwise Complex.real
let imag_array_c = elementwise Complex.imag

let elementwise_v f veclen n a = 
  array veclen (fun v ->
    array n (fun i -> f (a v i)))
let conj_v_array_c = elementwise_v Complex.conj
let real_v_array_c = elementwise_v Complex.real
let imag_v_array_c = elementwise_v Complex.imag


let hermitian_array_c n a =
  array n (fun i ->
    if (i = 0) then Complex.real (a 0)
    else if (i < n - i)  then (a i)
    else if (i > n - i)  then Complex.conj (a (n - i))
    else Complex.real (a i))

let transpose f i j = f j i
let symmetrize f i j = if i <= j then f i j else f j i

(* utilities for command-line parsing *)
let standard_arg_parse_fail _ = failwith "too many arguments"

let dump_dag alist =
  let fnam = !Magic.dag_dump_file in
  if (String.length fnam > 0) then
    let ochan = open_out fnam in
    begin
      To_alist.dump (output_string ochan) alist;
      close_out ochan;
    end

(* utilities for optimization *)
let standard_scheduler dag =
  let optim = Algsimp.algsimp dag in
  let alist = To_alist.to_assignments optim in
  let _ = dump_dag alist in
  let sched = Schedule.schedule alist in
  sched

let standard_optimizer dag =
  let sched = standard_scheduler dag in
  let annot = Annotate.annotate sched in
  annot


let size = ref None
let sign = ref (-1)

let speclist = [
  "-n", Arg.Int(fun i -> size := Some i), " generate a codelet of size <n>";
  "-sign",
  Arg.Int(fun i -> 
    if (i > 0) then
      sign := 1
    else
      sign := (-1)),
  " sign of transform";
]

let check_size () =
  match !size with
  | Some i -> i
  | None -> failwith "must specify -n"

let declare_register_fcn name =
  "void fftw_codelet_" ^ name ^ "(planner *p)\n"

let parse user_speclist usage =
  Arg.parse
    (user_speclist @ speclist @ Magic.speclist)
    standard_arg_parse_fail
    usage

let rec list_to_c = function
    [] -> ""
  | [a] -> (string_of_int a)
  | a :: b -> (string_of_int a) ^ ", " ^ (list_to_c b)

let rec list_to_comma = function
  | [a; b] -> C.Comma (a, b)
  | a :: b -> C.Comma (a, list_to_comma b)
  | _ -> failwith "list_to_comma"


type stride = Stride_variable | Fixed_int of int | Fixed_string of string

let either_stride a b =
  match a with
    Fixed_int x -> C.SInteger x
  | Fixed_string x -> C.SConst x
  | _ -> b

let stride_fixed = function
    Stride_variable -> false
  | _ -> true

let arg_to_stride s =
  try
    Fixed_int (int_of_string s)
  with Failure "int_of_string" -> 
    Fixed_string s

let stride_to_solverparm = function
    Stride_variable -> "0"
  | Fixed_int x -> string_of_int x
  | Fixed_string x -> x
