(*
 * Copyright (c) 1997-1999 Massachusetts Institute of Technology
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

(* various utility functions *)
open List
open Unix 

(*****************************************
 * Integer operations
 *****************************************)
(* fint the inverse of n modulo m *)
let invmod n m =
    let rec loop i =
	if ((i * n) mod m == 1) then i
	else loop (i + 1)
    in
	loop 1

(* Yooklid's algorithm *)
let rec gcd n m =
    if (n > m)
      then gcd m n
    else
      let r = m mod n
      in
	  if (r == 0) then n
	  else gcd r n

(* reduce the fraction m/n to lowest terms, modulo factors of n/n *)
let lowest_terms n m =
    if (m mod n == 0) then
      (1,0)
    else
      let nn = (abs n) in let mm = m * (n / nn)
      in let mpos = 
	  if (mm > 0) then (mm mod nn)
	  else (mm + (1 + (abs mm) / nn) * nn) mod nn
      and d = gcd nn (abs mm)
      in (nn / d, mpos / d)

(* find a generator for the multiplicative group mod p
   (where p must be prime for a generator to exist!!) *)

exception No_Generator

let find_generator p =
    let rec period x prod =
 	if (prod == 1) then 1
	else 1 + (period x (prod * x mod p))
    in let rec findgen x =
	if (x == 0) then raise No_Generator
	else if ((period x x) == (p - 1)) then x
	else findgen ((x + 1) mod p)
    in findgen 1

(* raise x to a power n modulo p (requires n > 0) (in principle,
   negative powers would be fine, provided that x and p are relatively
   prime...we don't need this functionality, though) *)

exception Negative_Power

let rec pow_mod x n p =
    if (n == 0) then 1
    else if (n < 0) then raise Negative_Power
    else if (n mod 2 == 0) then pow_mod (x * x mod p) (n / 2) p
    else x * (pow_mod x (n - 1) p) mod p

(******************************************
 * auxiliary functions 
 ******************************************)
let rec forall id combiner a b f =
    if (a >= b) then id
    else combiner (f a) (forall id combiner (a + 1) b f)

let sum_list l = fold_right (+) l 0
let max_list l = fold_right (max) l (-999999)
let min_list l = fold_right (min) l 999999
let count pred = fold_left (fun a elem -> if (pred elem) then 1 + a else a) 0

let remove elem = filter ((!=) elem)
let cons a b = a::b

let null = function [] -> true | _ -> false

(* functional composition *)
let (@@) f g x = f (g x)

(* Hmm... CAML won't allow second-order polymorphism.  Oh well.. *)
(* let forall_flat = forall (@);; *)
let rec forall_flat a b f = 
    if (a >= b) then []
    else (f a) @ (forall_flat (a + 1) b f)

let identity x = x

let find_elem p xs = try Some (List.find p xs) with Not_found -> None

(* find x, x >= a, such that (p x) is true *)
let rec suchthat a pred =
  if (pred a) then a else suchthat (a + 1) pred


let selectFirst p xs =
  let rec selectFirst' = function
    | [] -> raise Not_found
    | x::xs when p x -> (x,xs) 
    | x::xs -> let (x',xs') = selectFirst' xs in (x',x::xs')
  in try Some(selectFirst' xs) with Not_found -> None

(* used for inserting an element into a sorted list *)
let insertList stop el xs = 
  let rec insert' = function
    | [] -> [el]
    | x::xs as xxs -> if stop el x then el::xxs else x::(insert' xs)
  in insert' xs

(* used for inserting an element into a sorted list *)
let insert_list p el xs = 
  let rec insert' = function
    | [] -> [el]
    | x::xs as xxs -> if p el x < 0 then el::xxs else x::(insert' xs)
  in insert' xs

let zip xs = 
  let rec zip' ls rs = function
    | []    -> (ls,rs)
    | x::xs -> zip' (x::rs) ls xs
  in zip' [] [] xs

let rec intertwine xs zs = match (xs,zs) with
  | ([],zs) -> zs
  | (x::xs,zs) -> x::(intertwine zs xs)

  
let (@.) (a,b) (c,d) = (a@c,b@d)

let listAssoc key assoclist =
  try Some (List.assoc key assoclist) with Not_found -> None

let identity x = x

let listToString toString separator =
  let rec listToString_internal = function
    | [] -> ""
    | [x] -> toString x
    | x::xs -> (toString x) ^ separator ^ (listToString_internal xs) in
  listToString_internal

let stringlistToString = listToString identity

let intToString = string_of_int
let floatToString = string_of_float

let same_length xs zs = 
  let rec same_length_internal = function
    | [],[]  -> true
    | [], _  -> false
    | _, []  -> false
    | _::xs,_::zs -> same_length_internal (xs,zs) 
  in same_length_internal (xs,zs)

let optionIsSome = function None -> false | Some _ -> true
let optionIsNone = function None -> true  | Some _ -> false
let optionToValue' exn = function None -> raise exn | Some x -> x
let optionToValue v = optionToValue' (Failure "optionToValue") v
let optionToList = function None -> [] | Some a -> [a]

let optionToListAndConcat xs = function
  | None   -> xs
  | Some x -> x::xs

let option_to_boolvaluepair oldvalue = function
  | None 	  -> (false, oldvalue)
  | Some newvalue -> (true,  newvalue)

let minimize f xs =
  let rec minimize' z z' = function
    | []    -> Some z
    | x::xs -> 
        let x' = f x in
	  if x' < z' then minimize' x x' xs else minimize' z z' xs
  in match xs with
    | []    -> None
    | [x]   -> Some x
    | x::xs -> minimize' x (f x) xs

let list_removefirst p =
  let rec remove_internal = function
    | [] -> []
    | x::xs -> if p x then xs else x::(remove_internal xs)
  in remove_internal

let cons a b = a::b

let mapOption f = function
  | Some x -> Some (f x)
  | None   -> None

(*
use return/identity for that
let get1of1 x = x
*)

(*
use Pervasives.fst and Pervasives.snd for that
let get1of2 (x,_) = x
let get2of2 (_,x) = x
*)

let get1of3 (x,_,_) = x
let get2of3 (_,x,_) = x
let get3of3 (_,_,x) = x

let get1of4 (x,_,_,_) = x
let get2of4 (_,x,_,_) = x
let get3of4 (_,_,x,_) = x
let get4of4 (_,_,_,x) = x

let get1of5 (x,_,_,_,_) = x
let get2of5 (_,x,_,_,_) = x
let get3of5 (_,_,x,_,_) = x
let get4of5 (_,_,_,x,_) = x
let get5of5 (_,_,_,_,x) = x

let get1of6 (x,_,_,_,_,_) = x
let get2of6 (_,x,_,_,_,_) = x
let get3of6 (_,_,x,_,_,_) = x
let get4of6 (_,_,_,x,_,_) = x
let get5of6 (_,_,_,_,x,_) = x
let get6of6 (_,_,_,_,_,x) = x

let repl1of2 x (_,a) = (x,a)
let repl2of2 x (a,_) = (a,x)

let repl1of3 x (_,a,b) = (x,a,b)
let repl2of3 x (a,_,b) = (a,x,b)
let repl3of3 x (a,b,_) = (a,b,x)

let repl1of4 x (_,a,b,c) = (x,a,b,c)
let repl2of4 x (a,_,b,c) = (a,x,b,c)
let repl3of4 x (a,b,_,c) = (a,b,x,c)
let repl4of4 x (a,b,c,_) = (a,b,c,x)

let repl1of5 x (_,a,b,c,d) = (x,a,b,c,d)
let repl2of5 x (a,_,b,c,d) = (a,x,b,c,d)
let repl3of5 x (a,b,_,c,d) = (a,b,x,c,d)
let repl4of5 x (a,b,c,_,d) = (a,b,c,x,d)
let repl5of5 x (a,b,c,d,_) = (a,b,c,d,x)

let repl1of6 x (_,a,b,c,d,e) = (x,a,b,c,d,e)
let repl2of6 x (a,_,b,c,d,e) = (a,x,b,c,d,e)
let repl3of6 x (a,b,_,c,d,e) = (a,b,x,c,d,e)
let repl4of6 x (a,b,c,_,d,e) = (a,b,c,x,d,e)
let repl5of6 x (a,b,c,d,_,e) = (a,b,c,d,x,e)
let repl6of6 x (a,b,c,d,e,_) = (a,b,c,d,e,x)


let rec fixpoint f a = match f a with
  | (false, b) -> b
  | (true, b') -> fixpoint f b'

let return x = x

let diff a b = filter (fun x -> not (List.mem x b)) a

let addelem a set = if not (List.mem a set) then a :: set else set

let union l =
  let f x = addelem x   (* let is source of polymorphism *)
  in List.fold_right f l

let uniq l =
  List.fold_right (fun a b -> if List.mem a b then b else a :: b) l []

let msb x =
  let rec msb_internal msb0 = function
    | 0 -> msb0
    | n -> msb_internal (msb0+1) (n lsr 1) in
  msb_internal (-1) x

let lists_overlap xs zs = List.exists (fun i -> List.mem i xs) zs

let toNil _  = []
let toNone _ = None
let toZero _ = 0


(* print an information message *)
let info string =
  if !Magic.verbose then begin
    let now = Unix.times () 
    and pid = Unix.getpid () in
    prerr_string ((string_of_int pid) ^ ": " ^
		  "at t = " ^  (string_of_float now.tms_utime) ^ " : ");
    prerr_string (string ^ "\n");
    flush Pervasives.stderr;
  end

let debugOutputString str = 
  if !Magic.do_debug_output then Printf.printf "/* %s */\n" str else ()

let rec list_last = function
  | [] -> failwith "list_last"
  | [x] -> x
  | x::xs -> list_last xs

(*
 * freeze a function, i.e., compute it only once on demand, and
 * cache it into an array.
 *)
let array n f =
  let a = Array.init n (fun i -> lazy (f i))
  in fun i -> Lazy.force a.(i)

(* iota n produces the list [0; 1; ...; n - 1] *)
let iota n = forall [] cons 0 n identity

(* interval a b produces the list [a; 1; ...; b - 1] *)
let interval a b = List.map ((+) a) (iota (b - a))
