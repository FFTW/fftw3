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

(* This module features a set of rules to optimize the code produced by
 * the vectorizer. 
 *)


open List
open Util
open Id
open Number
open VSimdBasics
open VSimdIndexing

type istore' =
  | IStore of				(* INSTRUCTION STORE ***************)
	Id.t list VSimdRegMap.t * 	(*   table of readers (0..n)  	   *)
	Id.t list VSimdRegMap.t * 	(*   table of writers (0..1) 	   *)
	Id.t list VSICMap.t *		(*   all instrs in some category   *)
	vsimdinstr IdMap.t		(*   all instructions		   *)

open NonDetMonad

let fetchIStoreM    = fetchStateM >>= fun s -> unitM (fst s)
let fetchVisitedM   = fetchStateM >>= fun s -> unitM (snd s)

let storeIStoreM x  = fetchStateM >>= fun s -> storeStateM (repl1of2 x s)
let storeVisitedM x = fetchStateM >>= fun s -> storeStateM (repl2of2 x s)

(****************************************************************************)

let isUnusedM reg =
  fetchVisitedM >>= fun visited ->
    fetchIStoreM >>= fun (IStore(readers,_,_,_)) ->
      let readers_of_reg = vsimdregmap_findE reg readers in
        negAssertM (exists (fun i -> not (mem i visited)) readers_of_reg)

let idToInstrM id =
  fetchIStoreM >>= fun (IStore(_,_,_,idmap)) ->
    unitM (IdMap.find id idmap)

let addToVisitedM x = 
  fetchVisitedM >>= fun visited ->
    negAssertM (mem x visited) >>
      storeVisitedM (x::visited)

let markIdAsVisitedAndReturnIdInstrPairM mapM id =
  addToVisitedM id >>
    idToInstrM id >>= fun instr ->
      mapM instr >>= fun instr_info_tupel ->
	unitM (id,instr_info_tupel)

(****************************************************************************)

let getInstr1M mapM c =
  fetchIStoreM >>= fun (IStore(_,_,categories,_)) ->
    memberM (vsicmap_findE c categories) >>=
      markIdAsVisitedAndReturnIdInstrPairM mapM

let getInstr2M mapM cs = memberM cs >>= fun c -> getInstr1M mapM c

(****************************************************************************)

let copyinstrToTupelM = function
  | V_SimdUnaryOp(V_Id,s,d) -> unitM (s,d)
  | _ -> failM

let swapinstrToTupelM = function
  | V_SimdUnaryOp(V_Swap,s,d) -> unitM (s,d)
  | _ -> failM

let mulconstinstrToTupelM = function
  | V_SimdUnaryOp(V_MulConst(c1,c2),s,d) -> unitM (c1,c2,s,d)
  | _ -> failM

let chsloinstrToTupelM = function
  | V_SimdUnaryOp(V_Chs V_Lo,s,d) -> unitM (s,d)
  | _ -> failM

let chshiinstrToTupelM = function
  | V_SimdUnaryOp(V_Chs V_Hi,s,d) -> unitM (s,d)
  | _ -> failM

let unaryopinstrToTupelM = function
  | V_SimdUnaryOp(op,s,d) -> unitM (op,s,d)
  | _ -> failM

let chsinstrToTupelM = function
  | V_SimdUnaryOp(V_Chs p,s,d) -> unitM (p,s,d)
  | _ -> failM

let binopinstrToTupelM = function
  | V_SimdBinOp(op,s1,s2,d) -> unitM (op,s1,s2,d)
  | _ -> failM

let parbinopinstrToTupelM = function
  | V_SimdBinOp(op,s1,s2,d) when vsimdbinopIsParallel op -> unitM (op,s1,s2,d)
  | _ -> failM

let subinstrToTupelM = function
  | V_SimdBinOp(V_Sub,s1,s2,d) -> unitM (s1,s2,d)
  | _ -> failM

let addinstrToTupelM = function
  | V_SimdBinOp(V_Add,s1,s2,d) -> unitM (s1,s2,d)
  | _ -> failM

let mulinstrToTupelM = function
  | V_SimdBinOp(V_Mul,s1,s2,d) -> unitM (s1,s2,d)
  | _ -> failM

let nnaccinstrToTupelM = function
  | V_SimdBinOp(V_NNAcc,s1,s2,d) -> unitM (s1,s2,d)
  | _ -> failM

let npaccinstrToTupelM = function
  | V_SimdBinOp(V_NPAcc,s1,s2,d) -> unitM (s1,s2,d)
  | _ -> failM

(****************************************************************************)

let getUnaryopInstrM = 
  getInstr2M unaryopinstrToTupelM vsimdunaryopcategories_all

let getUnaryopInstrM_NoCopy = 
  getInstr2M unaryopinstrToTupelM vsimdunaryopcategories_nocopy

let getBinopInstrM = 
  getInstr2M binopinstrToTupelM vsimdbinopcategories_all

let getChsInstrM = 
  getInstr2M chsinstrToTupelM vsimdunaryopcategories_chs

let getXPAccInstrM = 
  getInstr2M binopinstrToTupelM [VSIC_BinPPAcc; VSIC_BinNPAcc]

let getParbinopInstrM = 
  getInstr2M parbinopinstrToTupelM vsimdbinopcategories_par

let getCopyInstrM = getInstr1M copyinstrToTupelM VSIC_UnaryCopy
let getSwapInstrM = getInstr1M swapinstrToTupelM VSIC_UnarySwap
let getMulconstInstrM = getInstr1M mulconstinstrToTupelM VSIC_UnaryMulConst
let getChsloInstrM = getInstr1M chsloinstrToTupelM VSIC_UnaryChsLo
let getChshiInstrM = getInstr1M chshiinstrToTupelM VSIC_UnaryChsHi

let getPXAccInstrM = getInstr1M binopinstrToTupelM VSIC_BinPPAcc
let getNNAccInstrM = getInstr1M nnaccinstrToTupelM VSIC_BinNNAcc
let getNPAccInstrM = getInstr1M npaccinstrToTupelM VSIC_BinNPAcc
let getAddInstrM = getInstr1M addinstrToTupelM VSIC_BinAdd
let getSubInstrM = getInstr1M subinstrToTupelM VSIC_BinSub
let getMulInstrM = getInstr1M mulinstrToTupelM VSIC_BinMul

(****************************************************************************)

let getReaderOfM mapM reg =
  fetchIStoreM >>= fun (IStore(readers,_,_,_)) ->
    memberM (VSimdRegMap.find reg readers) >>=
      markIdAsVisitedAndReturnIdInstrPairM mapM

let getWriterOfM mapM reg =
  fetchIStoreM >>= fun (IStore(_,writers,_,_)) ->
    memberM (VSimdRegMap.find reg writers) >>=
      markIdAsVisitedAndReturnIdInstrPairM mapM

let addInstrM instr = 
  let id = Id.makeNew ()
  and instr_category = vsimdinstrToCategory instr
  and (rs,ws) = vsimdinstrToReadswritespair instr in
    fetchIStoreM >>= fun (IStore(readers,writers,categories,idmap)) ->
      storeIStoreM (IStore(fold_right (vsimdregmap_addE' id) rs readers,
			   fold_right (vsimdregmap_addE' id) ws writers,
			   vsicmap_addE instr_category id categories,
			   IdMap.add id instr idmap))

(*********************************************************************)

let rec kickInstrWithIdM' id =
  let delId = list_removefirst (Id.equal id) in
  let del' k map = VSimdRegMap.add k (delId (VSimdRegMap.find k map)) map in
  fetchIStoreM >>= fun (IStore(readers,writers,categories,idmap)) ->
    let instr = IdMap.find id idmap in
      let (rs,ws) = vsimdinstrToReadswritespair instr
      and category = vsimdinstrToCategory instr in
        let c_entries = VSICMap.find category categories in
          storeIStoreM (IStore(fold_right del' rs readers,
			       fold_right del' ws writers,
			       VSICMap.add category 
					   (delId c_entries) 
					   categories,
			       IdMap.remove id idmap))
and kickInstrWithIdM id = 
  idToInstrM id >>= fun instr ->
    kickInstrWithIdM' id >>
      iterM maybeKickWriterOfM (vsimdinstrToReads instr)
and maybeKickWriterOfM reg =
  fetchIStoreM >>= fun (IStore(readers,writers,_,_)) ->
    match VSimdRegMap.find reg readers with
      | _::_ -> unitM ()
      | [] ->
          match VSimdRegMap.find reg writers with
	    | [] -> unitM () 		(* has already been kicked *)
	    | [x] -> 
		 let _ = debugOutputString "eliminating dead code" in
		   kickInstrWithIdM x
 	    | _ -> failwith "maybeKickWriterOfM (2)"

(*********************************************************************)

let doOneStepM rules = 
  memberM rules >>= fun (rule_description, rule_fn) ->
    rule_fn () >>= fun (to_be_kicked, to_be_added) ->
      let info_string = "firing optimization rule: " ^ rule_description in
      let _ = debugOutputString info_string in
	iterM addInstrM to_be_added >>
	  iterM kickInstrWithIdM to_be_kicked >>
	    fetchIStoreM

let istoreDoOnestep ruleset istore = 
  match runM doOneStepM ruleset (istore,[]) with
    | Some istore' -> (true,  istore')
    | None	   -> (false, istore)

let vsimdinstrsToIstore instrs = 
  let m = listToIdmap instrs in
    IStore(idmapToXs vsimdregmap_addE vsimdinstrToReads m VSimdRegMap.empty,
	   idmapToXs vsimdregmap_addE vsimdinstrToWrites m VSimdRegMap.empty,
   	   idmapToXs vsicmap_addE vsimdinstrToCategories m VSICMap.empty, m)

let istoreFixpoint (description,rules) = 
  info description;
  fixpoint (istoreDoOnestep rules)

let istoreToVsimdinstrs (IStore(_,_,_,m)) = IdMap.fold (fun _ -> cons) m [] 
let istoreToSize (IStore(_,_,_,m)) = IdMap.fold (fun _ _ -> succ) m 0

(*********************************************************************)

(* not all pairs are combinable *)
let vsimdunaryopCombineM = function
  | (V_Id, op) -> unitM op
  | (op, V_Id) -> unitM op
  | (V_Swap, V_Swap) -> unitM V_Id
  | (V_Chs V_Lo), (V_Chs V_Lo) -> unitM V_Id
  | (V_Chs V_Lo), (V_Chs V_Hi) -> unitM (V_Chs V_LoHi)
  | (V_Chs V_Lo), (V_Chs V_LoHi) -> unitM (V_Chs V_Hi)
  | (V_Chs V_Hi), (V_Chs V_Lo) -> unitM (V_Chs V_LoHi)
  | (V_Chs V_Hi), (V_Chs V_Hi) -> unitM V_Id
  | (V_Chs V_Hi), (V_Chs V_LoHi) -> unitM (V_Chs V_Lo)
  | (V_Chs V_LoHi), (V_Chs V_Lo) -> unitM (V_Chs V_Hi)
  | (V_Chs V_LoHi), (V_Chs V_Hi) -> unitM (V_Chs V_Lo)
  | (V_Chs V_LoHi), (V_Chs V_LoHi) -> unitM V_Id
  | (V_Chs V_Lo), (V_MulConst(n,m)) -> unitM (V_MulConst(negate n,m))
  | (V_Chs V_Hi), (V_MulConst(n,m)) -> unitM (V_MulConst(n,negate m))
  | (V_Chs V_LoHi), (V_MulConst(n,m)) -> unitM (V_MulConst(negate n, negate m))
  | (V_MulConst(n1,m1)), (V_MulConst(n2,m2)) ->
	unitM (V_MulConst(Number.mul n1 n2, Number.mul m1 m2))
  | (V_MulConst(n,m)), (V_Chs V_Lo) -> unitM (V_MulConst(negate n,m))
  | (V_MulConst(n,m)), (V_Chs V_Hi) -> unitM (V_MulConst(n, negate m))
  | (V_MulConst(n,m)), (V_Chs V_LoHi) -> unitM (V_MulConst(negate n, negate m))
  | _ -> failM

let substRegWithReg' map = function
  | V_SimdLoadQ(array,idx,d) -> 
      V_SimdLoadQ(array,idx,map d)
  | V_SimdLoadD(access,array,idx,pos,d) -> 
      V_SimdLoadD(access,array,idx,pos,map d)
  | V_SimdStoreQ(s,array,idx) -> 
      V_SimdStoreQ(map s,array,idx)
  | V_SimdStoreD(pos,s,access,array,idx) -> 
      V_SimdStoreD(pos,map s,access,array,idx)
  | V_SimdUnaryOp(op,s,d) -> 
      V_SimdUnaryOp(op,map s,map d)
  | V_SimdBinOp(op,s1,s2,d) -> 
      V_SimdBinOp(op,map s1,map s2,map d)

let substRegWithReg s d = substRegWithReg' (fun x -> if x=s then d else x)

(****************************************************************************)

let optimization_rules = 
    [
     "perform substitution",
     (fun _ ->
	getCopyInstrM >>= fun (_,(i1_s,i1_d)) ->
	  getReaderOfM unitM i1_d >>= fun (i2_id,i2_instr) ->
	    unitM ([i2_id], [substRegWithReg i1_d i1_s i2_instr]));

     "create substitution (for unaryop)",
     (fun _ ->
	getUnaryopInstrM_NoCopy >>= fun (i1_id,(i1_op,i1_s,i1_d)) ->
	  getReaderOfM unaryopinstrToTupelM i1_s >>= 
	    fun (i2_id,(i2_op,i2_s,i2_d)) ->
	      posAssertM (eq_vsimdunaryop i1_op i2_op) >>
	        unitM ([i2_id], [V_SimdUnaryOp(V_Id,i1_d,i2_d)]));

     "create substitution (for binop)",
     (fun _ ->
	getBinopInstrM >>= fun (i2_id,(i2_op,i2_s1,i2_s2,i2_d)) ->
	  getReaderOfM binopinstrToTupelM i2_s1 >>= 
	    fun (i1_id,(i1_op,i1_s1,i1_s2,i1_d)) ->
	      posAssertM (i1_op=i2_op && i1_s1=i2_s1 && i1_s2=i2_s2) >>
	        unitM ([i2_id], [V_SimdUnaryOp(V_Id,i1_d,i2_d)]));

     "combine unary operations",
     (fun _ ->
	getUnaryopInstrM_NoCopy >>= fun (i2_id,(i2_op,i2_s,i2_d)) ->
	  getWriterOfM unaryopinstrToTupelM i2_s >>= 
	    fun (i1_id,(i1_op,i1_s,i1_d)) ->
	      negAssertM (eq_vsimdunaryop V_Id i1_op) >>
	        vsimdunaryopCombineM (i1_op,i2_op) >>= fun combined_op ->
		  unitM ([i2_id], [V_SimdUnaryOp(combined_op,i1_s,i2_d)]));

     "accumulate-specific optimization 1 (for npacc, ppacc)",
     (fun _ ->
	getXPAccInstrM >>= fun (i2_id,(i2_op,i2_s1,i2_s2,i2_d)) ->
	  negAssertM (i2_s1 = i2_s2) >>
	  getWriterOfM swapinstrToTupelM i2_s2 >>= fun (i1_id,(i1_s,i1_d)) ->
	    unitM ([i2_id], [V_SimdBinOp(i2_op,i2_s1,i1_s,i2_d)]));

     "accumulate-specific optimization 2 (for ppacc)",
     (fun _ ->
	getPXAccInstrM >>= fun (i2_id,(i2_op,i2_s1,i2_s2,i2_d)) ->
	  getWriterOfM swapinstrToTupelM i2_s1 >>= fun (i1_id,(i1_s,i1_d)) ->
	    unitM ([i2_id], [V_SimdBinOp(i2_op,i1_s,i2_s2,i2_d)]));

     "accumulate-specific optimization 3 (for pnacc)",
     (fun _ ->
	getNPAccInstrM >>= fun (i2_id,(i2_s1,i2_s2,i2_d)) ->
	  isUnusedM i2_s1 >>
	  getWriterOfM swapinstrToTupelM i2_s1 >>= fun (i1_id,(i1_s,i1_d)) ->
	    posAssertM (i1_s = i2_s2) >>
	      unitM ([i2_id], [V_SimdBinOp(V_NPAcc,i2_s1,i2_s1,i2_d)]));

     "XXacc(op,A,B,C), swap(C,D) when is_unused(C) <=> XXacc(op,B,A,D)",
     (fun _ ->
	getSwapInstrM >>= fun (i2_id,(i2_s,i2_d)) -> 
	  isUnusedM i2_s >>
	    getWriterOfM binopinstrToTupelM i2_s >>= 
	      fun (i1_id,(i1_op,i1_s1,i1_s2,i1_d)) ->
	        posAssertM (i1_op = V_PPAcc || i1_op = V_NNAcc) >>
		  unitM ([i1_id; i2_id], 
			 [V_SimdBinOp(i1_op,i1_s2,i1_s1,i2_d)]));

     "swap+chslo+sub => swap+npacc+swap",
     (fun _ ->
	getChsloInstrM >>= fun (i2_id,(i2_s,i2_d)) ->
	  isUnusedM i2_s >>
	    getWriterOfM swapinstrToTupelM i2_s >>= fun (i1_id,(i1_s,i1_d)) ->
	      getReaderOfM subinstrToTupelM i1_s >>= 
	        fun (i3_id,(i3_s1,i3_s2,i3_d)) ->
		  posAssertM (i3_s1=i1_s && i3_s2=i2_d) >>
		    unitM ([i2_id; i3_id],
			   [V_SimdBinOp(V_NPAcc,i1_d,i1_d,i2_d);
			    V_SimdUnaryOp(V_Swap,i2_d,i3_d)]));

     "1. swap+chslo+add => npacc",
     (fun _ ->
	getChsloInstrM >>= fun (i2_id,(i2_s,i2_d)) ->
	  isUnusedM i2_s >>
	    getWriterOfM swapinstrToTupelM i2_s >>= fun (i1_id,(i1_s,i1_d)) ->
	      getReaderOfM addinstrToTupelM i2_d >>= 
	        fun (i3_id,(i3_s1,i3_s2,i3_d)) ->
		  isUnusedM i2_d >>
		    posAssertM (i3_s1=i1_s && i3_s2=i2_d || 
				i3_s2=i1_s && i3_s1=i2_d) >>
		      unitM ([i1_id; i2_id; i3_id], 
			     [V_SimdBinOp(V_NPAcc,i1_s,i1_s,i3_d)]));
			      
     "bfly2 @ swap+chshi+add ==> pnacc+swap",
     (fun _ ->
	getChshiInstrM >>= fun (i2_id,(i2_s,i2_d)) ->
	  getReaderOfM swapinstrToTupelM i2_s >>= fun (i1_id,(i1_s,i1_d)) ->
	    isUnusedM i2_s >>
	      getReaderOfM addinstrToTupelM i2_d >>= 
	        fun (i3_id,(i3_s1,i3_s2,i3_d)) ->
		  isUnusedM i2_d >>
		    posAssertM (i3_s1=i2_d && i3_s2=i1_d || 
				i3_s2=i1_d && i3_s2=i2_d) >>
		      unitM ([i1_id; i2_id; i3_id],
			     [V_SimdBinOp(V_NPAcc,i1_s,i1_s,i1_d);
			      V_SimdUnaryOp(V_Swap,i1_d,i3_d)]));

     "eliminate superfluous changesign instrs",
     (fun _ ->
	getChsloInstrM >>= fun (i1_id,(i1_s,i1_d)) ->
	  getReaderOfM chshiinstrToTupelM i1_s >>= fun (i2_id,(i2_s,i2_d)) ->
	  getReaderOfM addinstrToTupelM i1_d >>= fun (_,(i3_s1,i3_s2,i3_d)) ->
	    isUnusedM i1_d >>
	      getReaderOfM addinstrToTupelM i2_d >>= 
	        fun (i4_id,(i4_s1,i4_s2,i4_d)) ->
		  isUnusedM i2_d >>
		    let i4_othersrc = if i4_s1=i2_d then i4_s2 else i4_s1 in
		      unitM ([i2_id; i4_id],
			     [V_SimdBinOp(V_Sub,i4_othersrc,i1_d,i4_d)]));

     "swap,swap,nacc,mulconst => nacc,mulconst",
     (fun _ ->
	getNNAccInstrM >>= fun (i3_id,(i3_s1,i3_s2,i3_d)) ->
	  isUnusedM i3_s1 >>
	  isUnusedM i3_s2 >>
	    getWriterOfM swapinstrToTupelM i3_s1 >>= fun (i1_id,(i1_s,i1_d)) ->
	    getWriterOfM swapinstrToTupelM i3_s2 >>= fun (i2_id,(i2_s,i2_d)) ->
	      getReaderOfM mulconstinstrToTupelM i3_d >>= fun (i4_id, (i4_c1,i4_c2,i4_s,i4_d)) ->
		isUnusedM i3_d >>
		  unitM ([i1_id; i2_id; i3_id; i4_id],
			 [V_SimdBinOp(V_NNAcc,i1_s,i2_s,i3_d);
			  V_SimdUnaryOp(V_MulConst(negate i4_c1, negate i4_c2),
					i4_s,i4_d)]));

     "gen34 @ chshi+chslo+add-> sub+chshi",
     (fun _ ->
	getAddInstrM >>= fun (i3_id,(i3_s1,i3_s2,i3_d)) ->
	  isUnusedM i3_s1 >>
	  isUnusedM i3_s2 >>
	    getWriterOfM chsinstrToTupelM i3_s1 >>= 
	      fun (i1_id,(i1_chspos,i1_s,i1_d)) ->
	        getWriterOfM chsinstrToTupelM i3_s2 >>= 
		  fun (i2_id,(i2_chspos,i2_s,i2_d)) ->
	            (match (i1_chspos,i2_chspos) with
		       | (V_Hi, V_Lo) -> unitM V_Hi
		       | (V_Lo, V_Hi) -> unitM V_Lo
		       | _ -> failM) 
		    >>= fun pos ->
	 	      unitM ([i1_id; i2_id; i3_id],
		             [V_SimdBinOp(V_Sub,i1_s,i2_s,i2_d);
		              V_SimdUnaryOp(V_Chs pos,i2_d,i3_d)]));
   
     "gen35a @ nacc+chslo->swap+nacc",
     (fun _ ->
	getNNAccInstrM >>= fun (i1_id,(i1_s1,i1_s2,i1_d)) ->
	  getReaderOfM chsinstrToTupelM i1_d >>= 
	    fun (i2_id,(i2_pos,i2_s,i2_d)) ->
	      isUnusedM i2_s >>
	        (match i2_pos with
		   | V_Lo -> unitM ([i1_id; i2_id],
				    [V_SimdUnaryOp(V_Swap,i1_s1,i1_d);
			      	     V_SimdBinOp(V_NNAcc,i1_d,i1_s2,i2_d)])
		   | V_Hi -> unitM ([i1_id; i2_id],
				    [V_SimdUnaryOp(V_Swap,i1_s2,i1_d);
				     V_SimdBinOp(V_NNAcc,i1_s1,i1_d,i2_d)])
		   | _ -> failM));

     "up2/up3 @ chshi(A,B), swap(A,A') | chslo(A',B') <=> swap(B,B')",
     (fun _ ->
	getSwapInstrM >>= fun (i2_id,(i2_s,i2_d)) ->
	  getReaderOfM chsinstrToTupelM i2_d >>= 
	    fun (i3_id,(i3_chspos,i3_s,i3_d)) ->
	      isUnusedM i3_s >>
	      getReaderOfM chsinstrToTupelM i2_s >>= 
	 	fun (i1_id,(i1_chspos,i1_s,i1_d)) ->
	          posAssertM (i1_chspos=V_Hi && i3_chspos=V_Lo ||
			      i1_chspos=V_Lo && i3_chspos=V_Hi) >>
		    unitM ([i3_id], [V_SimdUnaryOp(V_Swap,i1_d,i3_d)]));

     "down5: swap+swap+parbinop ==> parbinop+swap",
     (fun _ ->
	getSwapInstrM >>= fun (i2_id,(i2_s,i2_d)) ->
	  getReaderOfM parbinopinstrToTupelM i2_d >>= 
	    fun (i3_id,(i3_op,i3_s1,i3_s2,i3_d)) ->
	      isUnusedM i2_d >>
	        posAssertM (i3_s2 = i2_d) >>
		  getWriterOfM swapinstrToTupelM i3_s1 >>= 
		    fun (i1_id,(i1_s,i1_d)) ->
		      unitM ([i2_id; i3_id],
			     [V_SimdBinOp(i3_op,i1_s,i2_s,i2_d);
			      V_SimdUnaryOp(V_Swap,i2_d,i3_d)]));

     "up12 @ mulconst,swap => swap,mulconst",
     (fun _ ->
	getSwapInstrM >>= fun (i2_id,(i2_s,i2_d)) ->
	  isUnusedM i2_s >>
	    getWriterOfM mulconstinstrToTupelM i2_s >>= 
	      fun (i1_id,(i1_c1,i1_c2,i1_s,i1_d)) ->
	        unitM ([i1_id; i2_id],
		       [V_SimdUnaryOp(V_Swap,i1_s,i1_d);
		        V_SimdUnaryOp(V_MulConst(i1_c2,i1_c1),i2_s,i2_d)]));

     "(TW) swap,mul,mul => swap,mul,mul",
     (fun _ ->
	getMulInstrM >>= fun (i2_id,(i2_s1,i2_s2,i2_d)) ->
	  isUnusedM i2_s2 >>
	  getWriterOfM swapinstrToTupelM i2_s2 >>= fun (i1_id,(i1_s,i1_d)) ->
	    getReaderOfM mulinstrToTupelM i1_s >>= 
	      fun (i3_id,(i3_s1,i3_s2,i3_d)) ->
	        isUnusedM i1_s >>
		  posAssertM (i3_s2=i1_s) >>
		    unitM ([i2_id; i3_id],
			   [V_SimdBinOp(V_Mul,i2_s2,i2_s1,i2_d);
			    V_SimdBinOp(V_Mul,i3_s2,i3_s1,i3_d)]));

     "DOWN6: swap+parbinop+swap ==> swap+parbinop",
     (fun _ ->
	getParbinopInstrM >>= fun (i2_id,(i2_op,i2_s1,i2_s2,i2_d)) ->
	  getWriterOfM swapinstrToTupelM i2_s1 >>= fun (i1_id,(i1_s,i1_d)) ->
	  getReaderOfM swapinstrToTupelM i2_d  >>= fun (i3_id,(i3_s,i3_d)) ->
	    isUnusedM i3_s >>
	      unitM ([i2_id; i3_id],
		     [V_SimdUnaryOp(V_Swap,i2_s2,i2_d);
		      V_SimdBinOp(i2_op,i1_s,i2_d,i3_d)]));

     "DOWN6B @ swap+parbinop+swap ==> swap+parbinop",
     (fun _ ->
	getParbinopInstrM >>= fun (i2_id,(i2_op,i2_s1,i2_s2,i2_d)) ->
	  getWriterOfM swapinstrToTupelM i2_s2 >>= fun (i1_id,(i1_s,i1_d)) ->
	  getReaderOfM swapinstrToTupelM i2_d  >>= fun (i3_id,(i3_s,i3_d)) ->
	    isUnusedM i3_s >>
	      unitM ([i2_id; i3_id],
		     [V_SimdUnaryOp(V_Swap,i2_s1,i2_d);
		      V_SimdBinOp(i2_op,i2_d,i1_s,i3_d)]));

    ]

(***************************************************************************)

let optimization_rules_pass1 = 
   ("optimization_pass1",

    optimization_rules @ 
    [
     "PASS1: swap,chs -> chs,swap",
     (fun _ ->
	getChsInstrM >>= fun (i2_id,(i2_pos,i2_s,i2_d)) ->
	  isUnusedM i2_s >>
	    getWriterOfM swapinstrToTupelM i2_s >>= fun (i1_id,(i1_s,i1_d)) ->
	      let pos' = vsimdposToSwapped i2_pos in
	        unitM ([i1_id; i2_id],
		       [V_SimdUnaryOp(V_Chs pos',i1_s,i1_d);
		        V_SimdUnaryOp(V_Swap,i2_s,i2_d)]));
    ]
   )

let optimization_rules_pass2 = 
   ("optimization_pass2",

    optimization_rules @ 
    [
     "PASS2: chs,swap -> swap,chs",
     (fun _ ->
	getSwapInstrM >>= fun (i2_id,(i2_s,i2_d)) ->
	  isUnusedM i2_s >>
	    getWriterOfM chsinstrToTupelM i2_s >>= 
	      fun (i1_id,(i1_pos,i1_s,i1_d)) ->
		let pos' = vsimdposToSwapped i1_pos in
	          unitM ([i1_id; i2_id],
		         [V_SimdUnaryOp(V_Swap,i1_s,i1_d);
		          V_SimdUnaryOp(V_Chs pos',i2_s,i2_d)]));
    ]
   )

let optimization_rules_k6 = 
   ("optimization_rules_k6",

    optimization_rules @
    [
     "K6: (1) swap -> unpacklo+unpackhi",
     (fun _ ->
	getSwapInstrM >>= fun (i1_id,(i1_s,i1_d)) ->
	  isUnusedM i1_s >>= fun _ ->
	    let tmp = makeNewVsimdreg () in
	      unitM ([i1_id],
		     [V_SimdBinOp(V_UnpckLo,i1_s,i1_s,tmp);
		      V_SimdBinOp(V_UnpckHi,i1_s,tmp,i1_d)]));

     "K6: (2) swap -> unpacklo+unpackhi",
     (fun _ ->
	getSwapInstrM >>= fun (i1_id,(i1_s,i1_d)) ->
	  let tmp = makeNewVsimdreg () in
	    unitM ([i1_id],
		   [V_SimdBinOp(V_UnpckHi,i1_s,i1_s,tmp);
		    V_SimdBinOp(V_UnpckLo,tmp,i1_s,i1_d)]));

     "K6: nnacc -> chshi+chshi+ppacc",
     (fun _ ->
	getNNAccInstrM >>= fun (i1_id,(i1_s1,i1_s2,i1_d)) ->
	  let tmp1 = makeNewVsimdreg () in
	  let tmp2 = makeNewVsimdreg () in
	    unitM ([i1_id],
		   [V_SimdBinOp(V_UnpckLo,i1_s1,i1_s2,tmp1);
		    V_SimdBinOp(V_UnpckHi,i1_s1,i1_s2,tmp2);
		    V_SimdBinOp(V_Sub,tmp1,tmp2,i1_d)]));

     "K6: npacc -> chshi+ppacc",
     (fun _ ->
	getNPAccInstrM >>= fun (i1_id,(i1_s1,i1_s2,i1_d)) ->
	  let tmp = makeNewVsimdreg () in
	    unitM ([i1_id],
		   [V_SimdUnaryOp(V_Chs (V_Hi),i1_s1,tmp);
		    V_SimdBinOp(V_PPAcc,tmp,i1_s2,i1_d)]));

    ]
   )

(***************************************************************************)

open Magic

(* exported *)
let apply_optrules instrs =
  let _ = Util.info "vK7Optimization.apply_optrules" in
  let istore0 = vsimdinstrsToIstore instrs in
  let istore1 = istoreFixpoint optimization_rules_pass1 istore0 in
  let istore' = istoreFixpoint optimization_rules_pass2 istore1 in
  let istore  = 
    if !target_processor = AMD_K6 then
      istoreFixpoint optimization_rules_k6 istore'
    else
      istore'
  in
    istoreToVsimdinstrs istore

