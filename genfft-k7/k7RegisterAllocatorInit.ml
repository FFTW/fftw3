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
open VSimdBasics
open K7Basics
open K7RegisterAllocationBasics

module HandleOnDemandInstructions : sig
  val addOndemandInstructions : 
	viregfileentry VIntRegMap.t -> k7vinstr list -> 
		viregfileentry VIntRegMap.t * K7Basics.k7vinstr list
end = struct
  open Util
  open StateMonad

  let fetchInstrsM    = fetchStateM >>= fun x -> unitM (fst x)
  let fetchVIRegFileM = fetchStateM >>= fun x -> unitM (snd x)

  let storeInstrsM x    = fetchStateM >>= fun s -> storeStateM (repl1of2 x s)
  let storeVIRegFileM x = fetchStateM >>= fun s -> storeStateM (repl2of2 x s)

  let addInstrM x = fetchInstrsM >>= consM x >>= storeInstrsM

  let rec addOndemandInstrs1M instr =
    iterM addOndemandInstrs2M (snd (k7vinstrToSrcvregs instr)) >>
      addInstrM instr
 
  and addOndemandInstrs2M vireg =
    fetchVIRegFileM >>= fun viregfile ->
      match vintregmap_find vireg viregfile with
        | Some (IOnDemand zs) -> 
	    storeVIRegFileM (VIntRegMap.add vireg IFresh viregfile) >>
	      iterM addOndemandInstrs1M zs
        | _ -> 
	    unitM ()

  let rec addOndemandInstrsM instrs =
    iterM addOndemandInstrs1M instrs >>
      fetchVIRegFileM >>= fun viregfile' ->
      fetchInstrsM >>= fun instrs' ->
	unitM (viregfile',List.rev instrs')

  let addOndemandInstructions viregfile instrs = 
    StateMonad.runM addOndemandInstrsM instrs ([],viregfile)
end

open HandleOnDemandInstructions

(****************************************************************************)

let processInitcode viregfile = function
  | AddIntOnDemandCode(reg,xs) -> VIntRegMap.add reg (IOnDemand xs) viregfile
  | _ -> viregfile

let makeInitialMap rregfile = function
  | FixRegister(vreg, rreg) -> (rreg, IFixed vreg)::(remove_assoc rreg rregfile)
  | _ -> rregfile

let addRegRefForInstr (n0,vsrefs0,virefs0) instr =
  let (_,_,(vsregs,viregs)) = k7vinstrToVregs instr in
  let vsrefs = fold_right (fun reg -> vsimdregmap_addE reg n0) vsregs vsrefs0
  and virefs = fold_right (fun reg -> vintregmap_addE reg n0) viregs virefs0 in
    (succ n0,vsrefs,virefs)

let getRegRefsForInstrs instrs =
  let s0 = (0,VSimdRegMap.empty,VIntRegMap.empty) in
  let (_,vsrefs0,virefs0) = fold_left addRegRefForInstr s0 instrs in
    (VSimdRegMap.map rev vsrefs0, VIntRegMap.map rev virefs0)

let addRegsForInstr i = (@.)(k7vinstrToDstvregs i @. k7vinstrToSrcvregs i) 

let prepareRegAlloc initcode instrs0 =
  let viregfile0 = fold_left processInitcode VIntRegMap.empty initcode
  and vsregfile0 = VSimdRegMap.empty in
  let (viregfile1, instrs) = addOndemandInstructions viregfile0 instrs0 in
  let (vsregs,viregs) = fold_right addRegsForInstr instrs ([],[]) in
  let rimap0 = map (fun reg -> (reg,IFree)) k7rintregs in
  let rimap1 = fold_left makeInitialMap rimap0 initcode in
  (((map (fun reg -> (reg,SFree)) k7rmmxregs), rimap1),
   (fold_right (fun reg -> VSimdRegMap.add reg SFresh) vsregs vsregfile0,
    fold_right (fun reg -> VIntRegMap.add reg IFresh) viregs viregfile1),
   getRegRefsForInstrs instrs,
   instrs)

