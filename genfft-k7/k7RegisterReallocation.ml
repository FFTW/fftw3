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

open List
open Util
open K7Basics
open StateMonad

(* In this module, SIMD registers are reallocated using a LRU heuristic.
 * All integer instructions remain unchanged. Also, SIMD register-spills
 * are not touched. *)

(****************************************************************************)

let fetchSubstsM   = fetchStateM >>= fun s -> unitM (get1of3 s)
let fetchFreeRegsM = fetchStateM >>= fun s -> unitM (get2of3 s)
let fetchInstrsM   = fetchStateM >>= fun s -> unitM (get3of3 s)

let storeSubstsM x   = fetchStateM >>= fun s -> storeStateM (repl1of3 x s)
let storeFreeRegsM x = fetchStateM >>= fun s -> storeStateM (repl2of3 x s)
let storeInstrsM x   = fetchStateM >>= fun s -> storeStateM (repl3of3 x s)

(****************************************************************************)

let nextReader reg annotatedinstrs = 
  let rec loop before_rev = function
    | [] -> None
    | ((_,_,instr) as x)::xs ->
	if k7rinstrReadsK7rmmxreg reg instr then
	  Some(before_rev,x,xs)
	else if k7rinstrWritesK7rmmxreg reg instr then
	  None
	else loop (x::before_rev) xs
  in loop [] annotatedinstrs

let availableReg reg instrs = optionIsNone (nextReader reg instrs)

let addInstrM instr = fetchInstrsM >>= consM instr >>= storeInstrsM

let freeRegM reg = fetchFreeRegsM >>= fun regs -> storeFreeRegsM (regs @ [reg])

let getSubstM reg = fetchSubstsM >>= fun m -> unitM (RSimdRegMap.find reg m)

let filterSubstM reg = 
  fetchSubstsM >>= fun map -> storeSubstsM (RSimdRegMap.remove reg map)

let addSubstM reg reg' = 
  fetchSubstsM >>= fun map -> storeSubstsM (RSimdRegMap.add reg reg' map)

let allocRegM x = 
  fetchFreeRegsM >>= function
    | []     -> failwith "allocRegM: cannot allocate register!"
    | x'::xs -> addSubstM x x' >> storeFreeRegsM xs >> unitM x'

let regMaybeDiesM reg reg' instrs = match nextReader reg instrs with
  | None   -> freeRegM reg' >> filterSubstM reg
  | Some _ -> unitM ()

(****************************************************************************)

let substRegsInK7rinstr map = function
  | K7R_SimdSpill(s,d) 		      -> K7R_SimdSpill(map s,d)
  | K7R_SimdLoad(size,addr,d)	      -> K7R_SimdLoad(size,addr,map d)
  | K7R_SimdStore(s,size,addr)	      -> K7R_SimdStore(map s,size,addr)
  | K7R_SimdBinOp(op,s,sd) 	      -> K7R_SimdBinOp(op,map s, map sd)
  | K7R_SimdBinOpMem(op,memop,sd)     -> K7R_SimdBinOpMem(op,memop,map sd)
  | K7R_SimdUnaryOp(op,sd) 	      -> K7R_SimdUnaryOp(op,map sd)
  | K7R_SimdCpyUnaryOp(op,s,d) 	      -> K7R_SimdCpyUnaryOp(op,map s,map d)
  | K7R_SimdCpyUnaryOpMem(op,memop,d) -> K7R_SimdCpyUnaryOpMem(op,memop,map d)
  | x -> x

let swap2regs r1 r2 (t,cplen,instr) =
  let map x = if x=r1 then r2 else if x=r2 then r1 else x in
    (t,cplen,substRegsInK7rinstr map instr)

(* checks that the consumer of s and the consumer of d also writes s resp. d *)
let hasShorterCPLen s d instrs = match nextReader s instrs with
  | Some(_,(_,cp_s,s_instr),_) when k7rinstrWritesK7rmmxreg s s_instr ->
      (match nextReader d instrs with
	 | Some(_,(_,cp_d,d_instr),_) when k7rinstrWritesK7rmmxreg d d_instr ->
	     cp_d > cp_s + 2
	 | _ -> false)
  | _ -> false

let chimovopt_applicable s d instrs =
  match nextReader d instrs with
    | Some(pre,(_,_,K7R_SimdBinOp(_,s',sd')),post) when s'<>sd' && sd'=d ->
	(match nextReader s' pre with
	   | None -> true
	   | Some(_,(_,_,K7R_SimdCpyUnaryOp(K7_FPSwap,x,y)),post') ->
		x=y &&
		optionIsNone (nextReader s' post') &&
		(Printf.printf "/* CHImovopt_applicable: new! */\n"; true)
	   | Some _ -> false) &&
	(match nextReader s' post with
	   | Some(_,(_,_,K7R_SimdBinOp(_,s'',sd'')),_) ->
	 	s''<>sd'' && s''=s && sd''=s'
	   | _ -> false)
    | _ -> false

let movswapopt_applicable s d instrs =
  match nextReader s instrs with
    | Some(_,(_,_,K7R_SimdCpyUnaryOp(K7_FPSwap,x,y)),_) -> x=y && x=s
    | _ -> false

(****************************************************************************)

(* requires stackcell-indices to be unique *)
let rec renameRegistersM = function
  | [] -> fetchInstrsM >>= fun instrs' -> unitM (List.rev instrs')
  | x::xs -> renameRegisters1M xs x

and renameRegisters1M instrs ((_,_,instr) as triple) = match instr with
  | K7R_IntLoadEA _
  | K7R_IntLoadMem _
  | K7R_IntStoreMem _
  | K7R_IntUnaryOp _
  | K7R_IntUnaryOpMem _
  | K7R_IntCpyUnaryOp _
  | K7R_IntBinOp _ 
  | K7R_IntBinOpMem _ 
  | K7R_Label _
  | K7R_Jump _
  | K7R_CondBranch _
  | K7R_FEMMS
  | K7R_Ret 
  | K7R_SimdPromiseCellSize _
  | K7R_SimdLoadStoreBarrier ->
      addInstrM instr >> 
	renameRegistersM instrs

  | K7R_SimdLoad(s_size,s_addr,d) ->
      allocRegM d >>= fun d' ->
	addInstrM (K7R_SimdLoad(s_size,s_addr,d')) >>
	  renameRegistersM instrs

  | K7R_SimdStore(s,d_size,d_addr) ->
      getSubstM s >>= fun s' ->
	addInstrM (K7R_SimdStore(s',d_size,d_addr)) >>
	  regMaybeDiesM s s' instrs >>
	    renameRegistersM instrs

  | K7R_SimdSpill(s,d_cellidx) ->
      getSubstM s >>= fun s' ->
        addInstrM (K7R_SimdSpill(s',d_cellidx)) >>
	  regMaybeDiesM s s' instrs >>
	    renameRegistersM instrs

  | K7R_SimdBinOp(op,s,sd) ->
      getSubstM s  >>= fun s' ->
      getSubstM sd >>= fun sd' ->
	addInstrM (K7R_SimdBinOp(op,s',sd')) >>
	  regMaybeDiesM s s' instrs >>
	    renameRegistersM instrs

  | K7R_SimdBinOpMem(op,s_memop,sd) ->
      getSubstM sd >>= fun sd' ->
        addInstrM (K7R_SimdBinOpMem(op,s_memop,sd')) >>
	  renameRegistersM instrs

  | K7R_SimdUnaryOp(op,sd) ->
      getSubstM sd >>= fun sd' ->
        addInstrM (K7R_SimdUnaryOp(op,sd')) >>
	  renameRegistersM instrs

  | K7R_SimdCpyUnaryOpMem(op,memop,d) ->
      let (pre,(t,cplen,instr),post) = optionToValue (nextReader d instrs) in
	(match instr with
	   | K7R_SimdBinOp(op,s,sd) when s=d &&	availableReg s post ->
	       let _ = debugOutputString "ld+binop ==> binopmem (1)" in
	       let instr' = K7R_SimdBinOpMem(op,memop,sd) in
		 renameRegistersM (rev_append pre ((t,cplen,instr')::post))

	   | K7R_SimdBinOp(op,s,sd) 
	     when sd=d && k7simdbinopIsParallel op && availableReg s post ->
	       let _ = debugOutputString "ld+binop ==> binopmem (2)" in
	       let instr' = K7R_SimdBinOpMem(k7simdbinopToParallel op,memop,s)
	       and post'  = map (swap2regs s sd) post in
		 renameRegistersM (rev_append pre ((t,cplen,instr')::post'))

	   | _ ->
	       allocRegM d >>= fun d' ->
		 addInstrM (K7R_SimdCpyUnaryOpMem(op,memop,d')) >>
		   renameRegistersM instrs)

  | K7R_SimdCpyUnaryOp(K7_FPId,s,d) when s=d ->
      let _ = debugOutputString "useless copy elimination (1)" in
	renameRegistersM instrs

  | K7R_SimdCpyUnaryOp(K7_FPId,s,d) when availableReg s instrs ->
      let _ = debugOutputString "useless copy elimination (2)" in
      getSubstM s >>= fun s' ->
	filterSubstM s >>
	addSubstM d s' >>
	  renameRegistersM instrs

  | K7R_SimdCpyUnaryOp(K7_FPId,s,d) 
    when chimovopt_applicable s d instrs || hasShorterCPLen s d instrs -> 
      let _ = debugOutputString "swapping operands of mov-instruction" in
	getSubstM s >>= fun s' ->
	allocRegM d >>= fun d' ->
	  addInstrM (K7R_SimdCpyUnaryOp(K7_FPId,s',d')) >>
	    addSubstM s d' >>
	    addSubstM d s' >>
	      renameRegistersM instrs

  | K7R_SimdCpyUnaryOp(K7_FPId,s,d) when movswapopt_applicable s d instrs ->
      let _ = debugOutputString "applying movswapopt" in
        getSubstM s >>= fun s' ->
	  allocRegM d >>= fun d' ->
	    addInstrM (K7R_SimdCpyUnaryOp(K7_FPSwap,s',d')) >>
	      addSubstM s d' >>
	      addSubstM d s' >>
		(match nextReader s instrs with
	           | Some(pre,_,post) -> renameRegistersM (rev_append pre post)
		   | _ -> failwith "movswapopt")

  | K7R_SimdCpyUnaryOp(K7_FPSwap,s,d) when s=d ->
      let _ = debugOutputString "swap opt (1)" in
      getSubstM s >>= fun s' ->
	addInstrM (K7R_SimdCpyUnaryOp(K7_FPSwap,s',s')) >>
	  renameRegistersM instrs

  | K7R_SimdCpyUnaryOp(K7_FPSwap,s,d) when availableReg s instrs ->
      let _ = debugOutputString "swap(s,d) ==> swap(s,s)" in
      getSubstM s >>= fun s' ->
	filterSubstM s >>
	addSubstM d s' >>
	addInstrM (K7R_SimdCpyUnaryOp(K7_FPSwap,s',s')) >>
	  renameRegistersM instrs

  | K7R_SimdCpyUnaryOp(op,s,d) ->
      getSubstM s >>= fun s' ->
      allocRegM d >>= fun d' ->
	addInstrM (K7R_SimdCpyUnaryOp(op,s',d')) >> 
	  renameRegistersM instrs


(* EXPORTED FUNCTIONS ******************************************************)

let k7rinstrsToRegisterReallocated annotatedinstrs = 
  runM renameRegistersM annotatedinstrs (RSimdRegMap.empty, k7rmmxregs, [])

