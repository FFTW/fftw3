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

(* This is the basic register allocator. *)


open List
open Util
open VSimdBasics
open K7Basics
open K7RegisterAllocationBasics
open K7RegisterAllocatorInit
open K7RegisterAllocatorEATranslation
open StateMonad


let fetchInstrsM         = fetchStateM >>= fun s -> unitM (get1of5 s)
let fetchRRegFilesM      = fetchStateM >>= fun s -> unitM (get2of5 s)
let fetchVRegFilesM      = fetchStateM >>= fun s -> unitM (get3of5 s)
let fetchFutureRefsM     = fetchStateM >>= fun s -> unitM (get4of5 s)
let fetchStackcellCntM   = fetchStateM >>= fun s -> unitM (get5of5 s)

let storeInstrsM x       = fetchStateM >>= fun s -> storeStateM (repl1of5 x s)
let storeRRegFilesM x    = fetchStateM >>= fun s -> storeStateM (repl2of5 x s)
let storeVRegFilesM x    = fetchStateM >>= fun s -> storeStateM (repl3of5 x s)
let storeFutureRefsM x   = fetchStateM >>= fun s -> storeStateM (repl4of5 x s)
let storeStackcellCntM x = fetchStateM >>= fun s -> storeStateM (repl5of5 x s)

let makeNewSimdStackcellM =
  fetchStackcellCntM >>= fun c0 -> storeStackcellCntM (succ c0) >> unitM c0

let addInstrM x = fetchInstrsM >>= consM x >>= storeInstrsM

let addSimdSpillInstrM rireg =
  makeNewSimdStackcellM >>= fun pos -> 
    addInstrM (K7R_SimdSpill(rireg,pos)) >>
      unitM pos

let addSimdReloadInstrM pos dst =
  addInstrM (K7R_SimdCpyUnaryOpMem(K7_FPId, 
				   (K7_MStackCell(K7_MMXStack,pos)), dst))

let addSimdCopyConstInstrM const dst =
  addInstrM (K7R_SimdCpyUnaryOpMem(K7_FPId,K7_MConst const,dst))

let addIntSpillInstrM reg = failwith "addIntSpillInstrM"
let addIntReloadInstrM pos dst = failwith "addIntReloadInstrM"
let addIntCopyConstInstrM c dst = failwith "addIntCopyConstInstrM"

(****************************************************************************)

let getVSRegFileEntryM vsreg =
  fetchVRegFilesM >>= fun (vsregfile,_) ->
    unitM (VSimdRegMap.find vsreg vsregfile)

let getRSRegFileEntryM rsreg =
  fetchRRegFilesM >>= fun (rsregfile,_) ->
    unitM (assoc rsreg rsregfile)

let setVSRegFileEntryM vsreg vsregfileentry =
  fetchVRegFilesM >>= fun (vsregfile,viregfile) ->
    storeVRegFilesM (VSimdRegMap.add vsreg vsregfileentry vsregfile,viregfile)

let setRSRegFileEntryM rsreg entry =
  fetchRRegFilesM >>= fun (rsregfile,riregfile) ->
    storeRRegFilesM ((rsreg,entry)::(remove_assoc rsreg rsregfile),riregfile)

let getVIRegFileEntryM vireg =
  fetchVRegFilesM >>= fun (_,viregfile) ->
    unitM (VIntRegMap.find vireg viregfile)

let getRIRegFileEntryM rireg =
  fetchRRegFilesM >>= fun (_,riregfile) ->
    unitM (assoc rireg riregfile)

let setVIRegFileEntryM vireg viregfileentry =
  fetchVRegFilesM >>= fun (vsregfile,viregfile) ->
    storeVRegFilesM (vsregfile, VIntRegMap.add vireg viregfileentry viregfile)

let setRIRegFileEntryM rireg entry =
  fetchRRegFilesM >>= fun (rsregfile,riregfile) ->
    storeRRegFilesM (rsregfile, (rireg,entry)::(remove_assoc rireg riregfile))


(* SPILL AND RELOAD SIMD REGISTERS ******************************************)

(* spill *)
let kickoutSimdM = function
  | SIn reg 	   -> addSimdSpillInstrM reg >>= fun p -> unitM (SOut p)
  | SInAndOut(i,p) -> unitM (SOut p)
  | SInConst(i,c)  -> unitM (SConst c)
  | _ 		   -> failwith "kickoutSimdM: Entry is not mapped!"

let addSimdSpillCodeM dst_rsreg dst_vsreg = function
  | [] -> unitM (SDying dst_rsreg)
  | _  -> getVSRegFileEntryM dst_vsreg >>= kickoutSimdM

let spillSimdM rsreg = function
  | SFree -> unitM ()
  | SHolds vsreg ->
      fetchFutureRefsM >>= fun (vsrefs,_) ->
	addSimdSpillCodeM rsreg vsreg (VSimdRegMap.find vsreg vsrefs) >>=
	  setVSRegFileEntryM vsreg

(* reload *)
let addSimdReloadCodeM dst = function
  | SFresh   -> unitM (SIn dst)
  | SOut pos -> addSimdReloadInstrM pos dst >> unitM (SInAndOut(dst,pos)) 
  | SConst c -> addSimdCopyConstInstrM c dst >> unitM (SInConst(dst,c)) 
  | _ 	     -> failwith "addSimdReloadCodeM: Invalid mapping!" 

let reloadSimdM dst_rsreg src_vsreg =
  getVSRegFileEntryM src_vsreg >>= 
    addSimdReloadCodeM dst_rsreg >>=
      setVSRegFileEntryM src_vsreg >>
      setRSRegFileEntryM dst_rsreg (SHolds src_vsreg)

(* choosing register to take *)

let vsregToTagged vsrefs all_vsregs reuse_dead_reg (rsreg,rsregfileentry) = 
  match rsregfileentry with
    | SFree -> [(rsreg,rsregfileentry,min_int)]
    | SHolds vsreg ->
	match VSimdRegMap.find vsreg vsrefs with
	  | [] when not reuse_dead_reg -> []
	  | [] -> [(rsreg,rsregfileentry,-1000000)]
	  | _::_ when mem vsreg all_vsregs -> []
	  | next_ref::_ -> [(rsreg,rsregfileentry,-next_ref)]

let getRSRegToBeUsedM vsrefs all_vsregs rsregfile mayReuseDeadRegister = 
  let choices = 
	concat (map (vsregToTagged 
			vsrefs all_vsregs mayReuseDeadRegister) rsregfile) in
  let (best_reg,best_entry,_) = optionToValue (minimize get3of3 choices) in
    unitM (best_reg,best_entry)


let spillAndReloadSimdM all_vsregs vsreg = 
  fetchRRegFilesM >>= fun (rsregfile,_) ->
  fetchFutureRefsM >>= fun (vsrefs,_) ->
  fetchVRegFilesM >>= fun (vsregfile,_) ->
    let mayReuseDeadRegister = 
		vsregfileentryIsFresh (VSimdRegMap.find vsreg vsregfile) in
      getRSRegToBeUsedM vsrefs 
			all_vsregs 
			rsregfile 
			mayReuseDeadRegister 
      >>= fun (dst_rsreg,dst_rregfileentry) ->
        spillSimdM dst_rsreg dst_rregfileentry >>
          reloadSimdM dst_rsreg vsreg


(* SPILL AND RELOAD INTEGER REGISTERS ***************************************)


(* spill *)
let kickoutIntM = function
  | IIn reg 	   -> addIntSpillInstrM reg >>= fun p -> unitM (IOut p)
  | IInAndOut(i,p) -> unitM (IOut p)
  | IInConst(i,c)  -> unitM (IConst c)
  | _ 		   -> failwith "kickoutIntM: Entry is not mapped!"

let addIntSpillCodeM dst_rireg dst_vireg = function
  | [] -> unitM (IDying dst_rireg)
  | _  -> getVIRegFileEntryM dst_vireg >>= kickoutIntM

let spillIntM rireg = function
  | IFree -> 
      unitM ()
  | IFixed _ ->
      unitM ()
  | ITmpHoldsProduct _ -> 
      unitM () 
  | IVarHoldsProduct(vireg,_,_) ->
      fetchFutureRefsM >>= fun (_,virefs) ->
	addIntSpillCodeM rireg vireg (VIntRegMap.find vireg virefs) >>=
	  setVIRegFileEntryM vireg
  | IHolds vireg ->
      fetchFutureRefsM >>= fun (_,virefs) ->
	addIntSpillCodeM rireg vireg (VIntRegMap.find vireg virefs) >>=
	  setVIRegFileEntryM vireg

(* reload *)
let addIntReloadCodeM dst = function
  | IFresh   -> unitM (IIn dst)
  | IOut pos -> addIntReloadInstrM pos dst >> unitM (IInAndOut(dst,pos))
  | IConst c -> addIntCopyConstInstrM c dst >> unitM (IInConst(dst,c))
  | _ 	     -> failwith "addIntReloadCodeM: Invalid mapping!"

let reloadIntM dst_rireg src_vireg =
  getVIRegFileEntryM src_vireg >>= 
    addIntReloadCodeM dst_rireg >>=
      setVIRegFileEntryM src_vireg >>
      setRIRegFileEntryM dst_rireg (IHolds src_vireg)

(* choosing register to use *)
let viregToTagged vreg virefs all_viregs reuse_dead_reg (rireg,riregfileentry) = 
  match riregfileentry with
  | IFixed v ->
      if (v = vreg) then
	[(rireg,riregfileentry,-100001)]
      else
	[(rireg,riregfileentry,100001)]
  | IFree -> 
      [(rireg,riregfileentry,-100000)]
  | ITmpHoldsProduct _ -> 
      [(rireg,riregfileentry,-100000)]
  | _ ->
      let vireg = optionToValue (riregfileentryToVireg riregfileentry) in
      (match VIntRegMap.find vireg virefs with
      | [] when not reuse_dead_reg -> []
      | [] -> [(rireg,riregfileentry,-1000000)]
      | _::_ when mem vireg all_viregs -> []
      | next_ref::_ -> [(rireg,riregfileentry,-next_ref)])


let getRIRegToBeUsedM vreg virefs all_viregs riregfile mayReuseDeadRegister = 
  let choices = 
	concat (map (viregToTagged 
			vreg virefs all_viregs mayReuseDeadRegister) riregfile) in
  let (best_rireg,best_regfileentry,_) = 
	optionToValue (minimize get3of3 choices) in
    unitM (best_rireg,best_regfileentry)

let spillAndReloadIntM all_viregs vireg = 
  fetchRRegFilesM >>= fun (_,riregfile) ->
    fetchFutureRefsM >>= fun (_,virefs) ->
    fetchVRegFilesM >>= fun (_,viregfile) ->
      let mayReuseDeadRegister = 
		viregfileentryIsFresh (VIntRegMap.find vireg viregfile) in
        getRIRegToBeUsedM 
	        vireg
		virefs 
		all_viregs 
		riregfile 
		mayReuseDeadRegister 
        >>= fun (dst_rireg,dst_rregfileentry) ->
          spillIntM dst_rireg dst_rregfileentry >>= fun _ ->
            reloadIntM dst_rireg vireg

(****************************************************************************)

let k7intunaryopToRiregfileentryscaling = function
  | K7_INegate 	 -> Some (-1)
  | K7_IShlImm x -> Some (1 lsl x)
  | _ 		 -> None

let k7intcpyunaryopToRiregfileentryscaling = function
  | K7_ICopy     -> Some 1
  | K7_IMulImm x -> Some x

let forceBufferedExprsOut' = function
  | IFree -> IFree
  | IHolds x -> IHolds x
  | IFixed x -> IFixed x
  | IVarHoldsProduct(x,_,_) -> IHolds x
  | ITmpHoldsProduct _ -> IFree

let forceBufferedExprsOut (reg,entry) = (reg,forceBufferedExprsOut' entry)

let forceUnusedExprsOut' virefs = function 
  | IFree -> IFree
  | IFixed x -> IFixed x
  | IHolds x -> if VIntRegMap.find x virefs = [] then IFree else IHolds x
  | IVarHoldsProduct(x,_,_) as orig -> 
      if VIntRegMap.find x virefs = [] then IFree else orig
  | ITmpHoldsProduct _ -> IFree

let forceUnusedExprsOut virefs (reg,entry) =
  (reg,forceUnusedExprsOut' virefs entry)

(* map a single instruction *)
let k7vinstrToK7rinstrs2M mapSimd mapInt = function
  | K7V_SimdLoadStoreBarrier -> 
      fetchFutureRefsM >>= fun (_,virefs) ->
        fetchRRegFilesM >>= fun (rsregfile,riregfile) ->
	  let riregfile' = map (forceUnusedExprsOut virefs) riregfile in
	  let (r_expensive,r_cheap) =
	    List.partition (fun (r,_) -> mem r k7rintregs_calleesaved)
			   riregfile' in
	  let riregfile'' = r_cheap @ r_expensive in
	    storeRRegFilesM (rsregfile,riregfile'') >>
	      unitM [K7R_SimdLoadStoreBarrier]

(*  | K7V_SimdLoadStoreBarrier -> 
      unitM [K7R_SimdLoadStoreBarrier]
*)
  | K7V_RefInts _ -> 
      unitM []
  | K7V_Label lbl -> 
      fetchRRegFilesM >>= fun (rsregfile,riregfile) ->
	storeRRegFilesM (rsregfile,map forceBufferedExprsOut riregfile) >>
	  unitM [K7R_Label lbl]
  | K7V_Jump (K7V_BTarget_Named s) ->
      unitM [K7R_Jump (K7R_BTarget_Named s)]
  | K7V_CondBranch(cond,K7V_BTarget_Named s)	  -> 
      unitM [K7R_CondBranch(cond, K7R_BTarget_Named s)]
  | K7V_IntLoadMem(s,d) -> 
      unitM [K7R_IntLoadMem(s, mapInt d)]
  | K7V_IntStoreMem(s,d) -> 
      unitM [K7R_IntStoreMem(mapInt s, d)]
  | K7V_SimdPromiseCellSize x ->
      unitM [K7R_SimdPromiseCellSize x]
  | K7V_IntUnaryOp(op,sd) ->
      let sd' = mapInt sd in
	getRIRegFileEntryM sd' >>= fun entry_sd' ->
	  setRIRegFileEntryM 
		sd' 
		(match (k7intunaryopToRiregfileentryscaling op, entry_sd') with
		   | (Some c, IHolds x) -> 
		       IHolds x
		   | (Some c, IFixed x) -> 
		       IFixed x
		   | (Some c, ITmpHoldsProduct(x,k)) -> 
		       ITmpHoldsProduct(x,c*k)
		   | (Some c, IVarHoldsProduct(v,x,k)) -> 
		       IVarHoldsProduct(v,x,c*k)
		   | (Some _, IFree) -> 
		       failwith "k7vinstrToK7rinstrs2M: IntUnaryOp"
		   | (None, _) -> 
		       IHolds sd) 
	  >>= fun _ ->
 	    unitM [K7R_IntUnaryOp(op,sd')]
  | K7V_IntCpyUnaryOp(op,s,d) -> 
      let (s',d') = (mapInt s, mapInt d) in
	getRIRegFileEntryM s' >>= fun entry_s' ->
	  setRIRegFileEntryM 
		d' 
		(match (k7intcpyunaryopToRiregfileentryscaling op, 
			entry_s') with
		   | (Some c, IHolds x) -> 
		 	IVarHoldsProduct(d,x,c)
		   | (Some c, ITmpHoldsProduct(x,k)) -> 
			ITmpHoldsProduct(x,c*k)
		   | (Some c, IVarHoldsProduct(v,x,k)) -> 
			IVarHoldsProduct(v,x,c*k)
		   | (Some _, _) -> 
			failwith "k7vinstrToK7rinstrs2M: IntCpyUnaryOp"
		   | (None, _) -> 
			IHolds d) 
	  >>= fun _ ->
	    unitM [K7R_IntCpyUnaryOp(op,s',d')]
  | K7V_IntUnaryOpMem(op,sd) -> 
      unitM [K7R_IntUnaryOpMem(op,sd)]
  | K7V_IntBinOp(op,s,sd) 	  -> 
      let sd' = mapInt sd in
	getRIRegFileEntryM sd' >>= fun entry_sd' ->
	  setRIRegFileEntryM 
		sd' 
		(match entry_sd' with
		   | IFree -> failwith "k7vinstrToK7rinstrs2M: IntBinOp"
		   | _ -> IHolds sd) 
	  >>= fun _ ->
	    unitM [K7R_IntBinOp(op,mapInt s,mapInt sd)]
  | K7V_IntBinOpMem(op,s,sd) -> 
      unitM [K7R_IntBinOpMem(op,s,mapInt sd)]
  | K7V_IntLoadEA(s,d) ->
      unitM [K7R_IntLoadEA(k7vaddrToK7raddr mapInt s,mapInt d)]
  | K7V_SimdUnaryOp(op,sd) -> 
      unitM [K7R_SimdUnaryOp(op,mapSimd sd)]
  | K7V_SimdCpyUnaryOp(op,s,d) -> 
      unitM [K7R_SimdCpyUnaryOp(op,mapSimd s,mapSimd d)]
  | K7V_SimdCpyUnaryOpMem(op,s,d) -> 
      unitM [K7R_SimdCpyUnaryOpMem(op,s,mapSimd d)]
  | K7V_SimdBinOp(op,s,sd) -> 
      unitM [K7R_SimdBinOp(op,mapSimd s,mapSimd sd)]
  | K7V_SimdBinOpMem(op,s,sd) ->
      unitM [K7R_SimdBinOpMem(op,s,mapSimd sd)]
  | K7V_SimdLoad(size,s,d) ->
      fetchRRegFilesM >>= fun (rsregfile,riregfile0) ->
	let (instrs,raddr,riregfile) = loadea_factor mapInt riregfile0 s in
	   storeRRegFilesM (rsregfile,riregfile) >>
	     unitM (instrs @ [K7R_SimdLoad(size,raddr,mapSimd d)])
  | K7V_SimdStore(s,size,d) ->
      fetchRRegFilesM >>= fun (rsregfile,riregfile0) ->
	let (instrs,raddr,riregfile) = loadea_factor mapInt riregfile0 d in
	  storeRRegFilesM (rsregfile,riregfile) >>
	    unitM (instrs @ [K7R_SimdStore(mapSimd s,size,raddr)])


let k7vinstrToK7rinstrsM instr = 
  fetchVRegFilesM >>= fun (vsregfile,viregfile) ->
    let mapSimd s = vsregfileentryToK7rmmxreg' (VSimdRegMap.find s vsregfile)
    and mapInt i  = viregfileentryToK7rintreg' (VIntRegMap.find i viregfile) in
      k7vinstrToK7rinstrs2M mapSimd mapInt instr


(* HANDLE FUTURE REFERENCES TO VSIMDREGS AND VINTREGS ***********************)

let maybeMarkVSRegAsDying vsrefs vsreg vsregfile =
  if VSimdRegMap.find vsreg vsrefs = [] then
    VSimdRegMap.add 
	vsreg 
	(SDying (optionToValue 
			(vsregfileentryToK7rmmxreg 
				(VSimdRegMap.find vsreg vsregfile)))) vsregfile
  else
    vsregfile

let maybeMarkVIRegAsDying virefs vireg viregfile = 
  if VIntRegMap.find vireg virefs = [] then
    VIntRegMap.add 
	vireg 
	(IDying (optionToValue 
			(viregfileentryToK7rintreg 
				(VIntRegMap.find vireg viregfile)))) viregfile 
  else
    viregfile
    

let adaptFutureRefsM (vsregs,viregs) =
  fetchFutureRefsM >>= fun (vsrefs0,virefs0) ->
    let vsrefs = fold_right vsimdregmap_cutE vsregs vsrefs0
    and virefs = fold_right vintregmap_cutE viregs virefs0 in
      storeFutureRefsM (vsrefs,virefs) >>
	fetchVRegFilesM >>= fun (vsregfile0,viregfile0) ->
	  let vsregfile = fold_right 
				(maybeMarkVSRegAsDying vsrefs) 
				vsregs 
				vsregfile0
	  and viregfile = fold_right 
				(maybeMarkVIRegAsDying virefs) 
				viregs 
				viregfile0 in
	    storeVRegFilesM (vsregfile,viregfile)


(* CLEANUP ******************************************************************)

let doCleanupSimd rsregfile0 vsregfile0 vsregs =
  let vsregfile = 
	fold_right 
	    (fun reg vsregfile ->
	       match vsimdregmap_find reg vsregfile with
		 | Some(SDying _) -> VSimdRegMap.remove reg vsregfile
		 | _ -> vsregfile) 
	    vsregs 
	    vsregfile0 in
  let (rsregs_live,rsregs_dead) = 
	partition 
	    (function
	       | (_,SFree) -> true
	       | (_,SHolds x) -> optionIsSome (vsimdregmap_find x vsregfile))
	    rsregfile0 in
    (rsregs_live @ (map (fun (x,_) -> (x,SFree)) rsregs_dead), vsregfile)

let doCleanupInt riregfile0 viregfile0 viregs =
  let viregfile = 
	fold_right 
	    (fun reg viregfile ->
	       match vintregmap_find reg viregfile with
	         | Some (IDying _) -> VIntRegMap.remove reg viregfile
		 | _ -> viregfile) 
	    viregs 
	    viregfile0 in
  let (riregs_live,riregs_dead) =
	partition 
	    (function
	       | (_,IFree) -> true
	       | (_,IFixed _) -> true
	       | (_,ITmpHoldsProduct _) -> true
	       | (_,IVarHoldsProduct(x,_,_)) ->
		   optionIsSome (vintregmap_find x viregfile)
	       | (_,IHolds x) ->
		   optionIsSome (vintregmap_find x viregfile))
	    riregfile0 in
     (riregs_live @ (map (fun (x,_) -> (x,IFree)) riregs_dead), viregfile)


let doCleanupM (all_vsregs,all_viregs) =
  fetchVRegFilesM >>= fun (vsregfile0,viregfile0) ->
    fetchRRegFilesM >>= fun (rsregfile0,riregfile0) ->
      let (rsregfile,vsregfile) = 
		doCleanupSimd rsregfile0 vsregfile0 all_vsregs in
      let (riregfile,viregfile) =
		doCleanupInt riregfile0 viregfile0 all_viregs in
        storeRRegFilesM (rsregfile,riregfile) >>
	  storeVRegFilesM (vsregfile,viregfile)

(****************************************************************************)

let touchDstRegsM (d_vsregs,d_viregs) =
  fetchVRegFilesM >>= fun (vsregfile,viregfile) ->
    storeVRegFilesM (fold_right touchSimdDstReg d_vsregs vsregfile,
		     fold_right touchIntDstReg d_viregs viregfile)

let doSpillsAndReloadsM (all_vsregs,all_viregs) (vsregs,viregs) =
  fetchVRegFilesM >>= fun (vsregfile,viregfile) ->
    let viregs_need_reg = filter (viregWantsIn viregfile) viregs 
    and vsregs_need_reg = filter (vsregWantsIn vsregfile) vsregs in
      iterM (spillAndReloadIntM  all_viregs) viregs_need_reg >>
      iterM (spillAndReloadSimdM all_vsregs) vsregs_need_reg >>
	adaptFutureRefsM (vsregs,viregs)

let regalloc1M instr =
  let (s_vregs,d_vregs,all_vregs) = k7vinstrToVregs instr in
    doSpillsAndReloadsM all_vregs s_vregs >>
    doSpillsAndReloadsM all_vregs d_vregs >>
      k7vinstrToK7rinstrsM instr >>=
	iterM addInstrM >>
	  touchDstRegsM (k7vinstrToDstvregs instr) >>
	    doCleanupM all_vregs

let regallocM instrs = 
  iterM regalloc1M instrs >>
    fetchInstrsM >>= fun instrs_rev ->
      unitM (rev instrs_rev)

(****************************************************************************)

let regalloc initcode instrs0 = 
  let (rregfiles,vregfiles,vrefs,instrs) = prepareRegAlloc initcode instrs0 in
    runM regallocM instrs ([],rregfiles,vregfiles,vrefs,0)
