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

(* This module contains code for the so-called 'null-vectorization'. *)

open List
open Util
open Expr
open Variable
open Number
open VFpBasics
open VSimdBasics
open K7Basics
open StateMonad
open MemoMonad
open BalanceVfpinstrs

(* monadic access functions *)
let fetchRegsM   = fetchStateM >>= fun s -> unitM (fst s)
let fetchInstrsM = fetchStateM >>= fun s -> unitM (snd s)

let storeRegsM x   = fetchStateM >>= fun s -> storeStateM (repl1of2 x s)
let storeInstrsM x = fetchStateM >>= fun s -> storeStateM (repl2of2 x s)

let addInstrM x = fetchInstrsM >>= consM x >>= storeInstrsM

(****************************************************************************)

let lookupRegM k = fetchRegsM >>= fun m -> unitM (vfpregmap_find k m)
let insertRegM k v = fetchRegsM >>= fun m -> storeRegsM (VFPRegMap.add k v m)
let makeNewRegM _ = unitM (makeNewVsimdreg ())
let mapRegM = memoizingM lookupRegM insertRegM makeNewRegM

(****************************************************************************)

let rec nullVectorize1M = function
  | V_FPLoad(s_access,s_array,s_idx,d) ->
      mapRegM d >>= fun d' ->
	addInstrM (V_SimdLoadD(s_access,s_array,s_idx,V_Lo,d'))
  | V_FPStore(s,d_access,d_array,d_idx) ->
      mapRegM s >>= fun s' ->
	addInstrM (V_SimdStoreD(V_Lo,s',d_access,d_array,d_idx))
  | V_FPUnaryOp(op,s,d) ->
      mapPairM mapRegM (s,d) >>= fun (s',d') ->
	addInstrM (V_SimdUnaryOp(scalarvfpunaryopToVsimdunaryop op V_Lo,s',d'))
  | V_FPBinOp(op,s1,s2,d) ->
      mapTripleM mapRegM (s1,s2,d) >>= fun (s1',s2',d') ->
	addInstrM (V_SimdBinOp(vfpbinopToVsimdbinop op,s1',s2',d'))
  | V_FPAddL(xs,d) ->
      failwith "nullVectorize1M: V_FPAddL _ not supported!"

let nullVectorizeM instrs = iterM nullVectorize1M instrs >> fetchInstrsM 

(****************************************************************************)

let nullVectorize n vinstrs =
  info "doing null-vectorization";
  debugOutputString "last resort: using grade VECTGRADE_NULL";
  let vinstrs' = concat (map fullyexpandaddlistVfpinstr vinstrs) in
    (K7_DWord, MemoMonad.runM (VFPRegMap.empty,[]) nullVectorizeM vinstrs')


