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
(* $Id: twiddle.ml,v 1.9 2002-08-18 20:02:37 athena Exp $ *)

(* policies for loading/computing twiddle factors *)
open Complex
open Util

type twop = TW_FULL | TW_COS | TW_SIN | TW_TAN | TW_NEXT

let optoint = function
  | TW_COS -> 0
  | TW_SIN -> 1
  | TW_TAN -> 2
  | TW_NEXT -> 3
  | TW_FULL -> 4

let optostring = function
  | TW_COS -> "TW_COS"
  | TW_SIN -> "TW_SIN"
  | TW_TAN -> "TW_TAN"
  | TW_NEXT -> "TW_NEXT"
  | TW_FULL -> "TW_FULL"

type twinstr = (twop * int * int)

let rec unroll_twfull l = match l with
| [] -> []
| (TW_FULL, 0, n) :: b ->
    (List.flatten
       (forall [] cons 1 n (fun i -> [(TW_COS, 0, i); (TW_SIN, 0, i)])))
    @ unroll_twfull b
| a :: b -> a :: unroll_twfull b

let twinstr_to_c_string l =
  let one (op, a, b) = Printf.sprintf "{ %s, %d, %d }" (optostring op) a b
  in let rec loop first = function
    | [] -> ""
    | a :: b ->  (if first then "\n" else ",\n") ^ (one a) ^ (loop false b)
  in "{" ^ (loop true l) ^ "}"

let twinstr_to_simd_string l =
  let one sep = function
    | (TW_NEXT, 1, 0) -> sep ^ "{TW_NEXT, VL, 0}"
    | (TW_NEXT, _, _) -> failwith "twinstr_to_simd_string"
    | (TW_COS, 0, b) -> sep ^ (Printf.sprintf "VTW(%d)" b)
    | (TW_SIN, 0, b) -> ""
    | _ -> failwith "twinstr_to_simd_string"
  in let rec loop first = function
    | [] -> ""
    | a :: b ->  (one (if first then "\n" else ",\n") a) ^ (loop false b)
  in "{" ^ (loop true (unroll_twfull l)) ^ "}"
  
let square x = 
  if (!Magic.wsquare) then
    wsquare x
  else
    times x x

let rec pow m n =
  if (n = 0) then 1
  else m * pow m (n - 1)

let rec is_pow m n =
  n = 1 || ((n mod m) = 0 && is_pow m (n / m))

let rec log m n = if n = 1 then 0 else 1 + log m (n / m)

let rec largest_power_smaller_than m i =
  if (is_pow m i) then i
  else largest_power_smaller_than m (i - 1)

let rec smallest_power_larger_than m i =
  if (is_pow m i) then i
  else smallest_power_larger_than m (i + 1)

let rec_array n f =
  let g = ref (fun i -> Complex.zero) in
  let a = Array.init n (fun i -> lazy (!g i)) in
  let h i = f (fun i -> Lazy.force a.(i)) i in
  begin
    g := h;
    h
  end

 
let load_reim sign w i = 
  if sign = 1 then w i else Complex.conj (w i)

(* various policies for computing/loading twiddle factors *)

(* addition chain: given a set s of numbers to be stored, and the cost
   of multiplications and squarings, compute the optimal solution
   via dynamic programming 
 *)
   
let addition_chain n s load cost_load cost_mul cost_sq =
  let infty = 100000 in
  let solution = Array.init n (fun i () -> failwith "addition_chain") in
  let solution = Array.init n (fun i () -> failwith (string_of_int i)) in
  let costs = Array.init n (fun i -> infty) in
  let changed = ref false in
  let update_one i j k how =
    if (k >= 0 && k < n) then
      let c = if i = j then cost_sq else cost_mul in
      let d = c + costs.(i) + costs.(j) in
      if (d < costs.(k)) then
	begin
	  costs.(k) <- d;
	  solution.(k) <- how;
	  changed := true;
	end
  in
  let update_all () = 
    begin
      changed := false;
      for i = 0 to n - 1 do
	for j = 0 to n - 1 do
	  begin
	    update_one i j (i + j)
	      (fun () -> Complex.times (solution.(i) ()) (solution.(j) ()));
	    update_one i j (i - j)
	      (fun () -> 
		Complex.times (solution.(i) ()) 
		  (Complex.conj (solution.(j) ())));
	  end;
	done
      done
    end;
  in
  let init () =
    begin
      costs.(0) <- 0; (* always free *)
      solution.(0) <- (fun () -> Complex.one);
      List.iter2 (fun k i -> 
	costs.(k) <- cost_load;
	solution.(k) <- (fun () -> load i))
	s (iota (List.length s));
    end
  in begin
    init ();
    changed := true;
    while !changed do update_all () done;
    fun i -> solution.(i) ();
  end
  
   

(* load all twiddle factors *)
let twiddle_policy_load_all =
  let bytwiddle n sign w f i =
    if i = 0 then 
      f i
    else
      Complex.times (load_reim sign w (i - 1)) (f i)
  and twidlen n = 2 * (n - 1)
  and twdesc r = [(TW_FULL, 0, r);(TW_NEXT, 1, 0)]
  in bytwiddle, twidlen, twdesc

let twiddle_policy_addition_chain t =
  let load_set n = match (n, t) with
    | (4, 1) -> [1;] (* 20 *)
    | (4, 2) -> [1;3;] (* 8 *)
    | (4, _) -> [1;2;3;] (* 3 *)
    | (8, 1) -> [1;] (* 122 *)
    | (8, 2) -> [2;3;] (* 43 *)
    | (8, 3) -> [1;3;4;] (* 31 *)
    | (8, 4) -> [1;2;3;5;] (* 24 *)
    | (8, 5) -> [1;2;3;5;7;] (* 17 *)
    | (16, 1) -> [1;] (* 574 *)
    | (16, 2) -> [3;5;] (* 162 *)
    | (16, 3) -> [4;5;7;] (* 108 *)
    | (16, 4) -> [1;3;7;12;] (* 86 *)
    | (16, 5) -> [1;3;4;7;12;] (* 77 *)
    | (32, 1) -> [1;] (* 2454 *)
    | (32, 2) -> [5;6;] (* 524 *)
    | (32, 3) -> [7;8;10;] (* 343 *)
    | (32, 4) -> [6;11;14;15;] (* 271 *)
    | (32, 5) -> [5;8;12;14;15;] (* 228 *)
    | (64, 1) -> [1;] (* 10086 *)
    | (64, 2) -> [7;9;] (* 1614 *)
    | (64, 3) -> [11;12;19;] (* 975 *)
    | (64, 4) -> [11;17;20;21;] (* 756 *)
    | (64, 5) -> [3;17;18;28;30;] (* 644 *)
    | _ -> failwith "unknown addition chain"
  in
  let bytwiddle n sign w f =
    let g = addition_chain n (load_set n) (load_reim sign w) 1 6 4 in
    fun i -> Complex.times (g i) (f i)    
  and twidlen n = 2 * List.length (load_set n)
  and twdesc r = 
    List.flatten (List.map (fun i -> [(TW_COS, 0, i); (TW_SIN, 0, i)])
		    (load_set r))
    @ [(TW_NEXT, 1, 0)]
  in bytwiddle, twidlen, twdesc

(*
 * if i is a power of two, then load w (log i)
 * else let x = largest power of 2 less than i in
 *      let y = i - x in
 *      compute w^{x+y} = w^x * w^y
 *)
let twiddle_policy_log2 =
  let bytwiddle n sign w f =
    let g = rec_array n (fun self i ->
      if i = 0 then Complex.one
      else if is_pow 2 i then load_reim sign w (log 2 i)
      else let x = largest_power_smaller_than 2 i in
      let y = i - x in
      Complex.times (self x) (self y))
    in fun i -> Complex.times (g i) (f i)
  and twidlen n = 2 * (log 2 (largest_power_smaller_than 2 (2 * n - 1)))
  and twdesc n =
    (List.flatten 
      (List.map 
	 (fun i -> 
	   if i > 0 && is_pow 2 i then 
	     [(TW_COS, 0, i); (TW_SIN, 0, i)] 
	   else 
	     [])
	 (iota n)))
      @ [(TW_NEXT, 1, 0)]
  in bytwiddle, twidlen, twdesc

let twiddle_policy_log3 =
  let rec terms_needed i pi s n =
    if (s >= n - 1) then i
    else terms_needed (i + 1) (3 * pi) (s + pi) n
  in
  let rec bytwiddle n sign w f =
    let g = rec_array (3 * n) (fun self i ->
      if i = 0 then Complex.one
      else if is_pow 3 i then load_reim sign w (log 3 i)
      else let x = smallest_power_larger_than 3 i in
      if (i + i >= x) then
	Complex.times (self x) (Complex.conj (self (x - i)))
      else let x = largest_power_smaller_than 3 i in
      Complex.times (self x) (self (i - x)))
    in fun i -> Complex.times (g i) (f i)
  and twidlen n = 2 * (terms_needed 0 1 0 n)
  and twdesc n =
    (List.flatten 
      (List.map 
	 (fun i -> [(TW_COS, 0, (pow 3 i)); (TW_SIN, 0, (pow 3 i))])
	 (iota ((twidlen n) / 2))))
      @ [(TW_NEXT, 1, 0)]
  in bytwiddle, twidlen, twdesc


(* shorthand for policies that only load W[0] *)
let policy_one mktw =
  let bytwiddle n sign w f = 
    let g = (mktw n (load_reim sign w)) in
    fun i -> Complex.times (g i) (f i)
  and twidlen n = 2
  and twdesc n = [(TW_COS, 0, 1); (TW_SIN, 0, 1); (TW_NEXT, 1, 0)]
  in bytwiddle, twidlen, twdesc
    
(* compute w^n = w w^{n-1} *)
let twiddle_policy_iter =
  policy_one (fun n ltw ->
    rec_array n (fun self i ->
      if i = 0 then Complex.one
      else if i = 1 then ltw (i - 1)
      else times (self (i - 1)) (self 1)))

(*
 * if n = 2, compute w^n = (w^{n/2})^2, else
 * compute  w^n from w^{n-1}, w^{n-2}, and w
 *)
let twiddle_policy_iter2 =
  policy_one (fun n ltw ->
    rec_array n (fun self i ->
      if i = 0 then Complex.one
      else if i = 1 then ltw (i - 1)
      else if i = 2 then
	square (self (i / 2))
      else 
	wreflects (self (i - 2)) (self (i - 1)) (self 1)))


(*
 * if n is even, compute w^n = (w^{n/2})^2, else
 *  w^n = w w^{n-1}
 *)
let twiddle_policy_square1 =
  policy_one (fun n ltw ->
    rec_array n (fun self i ->
      if i = 0 then Complex.one
      else if i = 1 then ltw (i - 1)
      else if ((i mod 2) == 0) then
	square (self (i / 2))
      else times (self (i - 1)) (self 1)))

(*
 * if n is even, compute w^n = (w^{n/2})^2, else
 * compute  w^n from w^{n-1}, w^{n-2}, and w
 *)
let twiddle_policy_square2 =
  policy_one (fun n ltw ->
    rec_array n (fun self i ->
      if i = 0 then Complex.one
      else if i = 1 then ltw (i - 1)
      else if ((i mod 2) == 0) then
	square (self (i / 2))
      else 
	wreflectc (self (i - 2)) (self (i - 1)) (self 1)))

(*
 * if n is even, compute w^n = (w^{n/2})^2, else
 *  w^n = w^{floor(n/2)} w^{ceil(n/2)}
 *)
let twiddle_policy_square3 =
  policy_one (fun n ltw ->
    rec_array n (fun self i ->
      if i = 0 then Complex.one
      else if i = 1 then ltw (i - 1)
      else if ((i mod 2) == 0) then
	square (self (i / 2))
      else times (self (i / 2)) (self (i - i / 2))))

(*
 * twiddle policy used by Takuya Ooura in his code.
 * if i is a power of two, then w^i = (w^(i/2)) ^2
 * else let x = largest power of 2 less than i in
 *      let y = i - x in
 *      compute w^{x+y} from w^{x-y}, w^{x}, and w^{y} using
 *      sine reflection formula
 *)
let twiddle_policy_ooura =
  policy_one (fun n ltw ->
    rec_array n (fun self i ->
      if i = 0 then Complex.one
      else if (i == 1) then ltw (i - 1)
      else if (is_pow 2 i) then
	square (self (i / 2))
      else
	let x = largest_power_smaller_than 2 i in
	let y = i - x in
	wreflects (self (x - y)) (self x) (self y)))

let current_twiddle_policy = ref twiddle_policy_load_all

let twiddle_policy () = !current_twiddle_policy

let set_policy x = Arg.Unit (fun () -> current_twiddle_policy := x)
let set_policy_int x = Arg.Int (fun i -> current_twiddle_policy := x i)

let undocumented = " Undocumented twiddle policy"

let speclist = [
  "-twiddle-load-all", set_policy twiddle_policy_load_all, undocumented;
  "-twiddle-log2", set_policy twiddle_policy_log2, undocumented;
  "-twiddle-log3", set_policy twiddle_policy_log3, undocumented;
  "-twiddle-iter", set_policy twiddle_policy_iter, undocumented;
  "-twiddle-iter2", set_policy twiddle_policy_iter2, undocumented;
  "-twiddle-square1", set_policy twiddle_policy_square1, undocumented;
  "-twiddle-square2", set_policy twiddle_policy_square2, undocumented;
  "-twiddle-square3", set_policy twiddle_policy_square3, undocumented;
  "-twiddle-ooura", set_policy twiddle_policy_ooura, undocumented;
  "-twiddle-addchain", set_policy_int twiddle_policy_addition_chain, 
  undocumented;
] 
