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

(* This module contains code for the 2-way SIMD vectorization. *)


open Id
open List 
open Util
open Expr
open Variable
open Number
open VFpBasics
open VFpUnparsing
open BalanceVfpinstrs
open VSimdBasics
open VSimdUnparsing
open K7Basics 

exception OutOfTime

type fpfuse =
  | ReImSimd of vfpreg * vfpreg * vsimdreg
  | ImReSimd of vfpreg * vfpreg * vsimdreg

type vect_instr_tupel =
  | SingleVect of (Id.t * vfpinstr)
  | DualVect of vsimdreg * (Id.t * vfpinstr) * (Id.t * vfpinstr)

type fusemode = 
  | FuseModeSrc
  | FuseModeDst

type vectorization_grade =
  | VECTGRADE_FULLH0
  | VECTGRADE_FULLH1
  | VECTGRADE_FULLH2
  | VECTGRADE_FULL
  | VECTGRADE_SEMIH1
  | VECTGRADE_SEMI

let vectorizationgradeToString = function
  | VECTGRADE_FULLH0 -> "VECTGRADE_FULLH0"
  | VECTGRADE_FULLH1 -> "VECTGRADE_FULLH1"
  | VECTGRADE_FULLH2 -> "VECTGRADE_FULLH2"
  | VECTGRADE_FULL   -> "VECTGRADE_FULL"
  | VECTGRADE_SEMIH1 -> "VECTGRADE_SEMIH1"
  | VECTGRADE_SEMI   -> "VECTGRADE_SEMI"

let vectorization_grades = 
  [
   VECTGRADE_FULLH0; VECTGRADE_FULLH1; VECTGRADE_FULLH2; VECTGRADE_FULL;
   VECTGRADE_SEMIH1; VECTGRADE_SEMI; 
  ]

let transformlength = ref 0
let current_vectorization_grade = ref VECTGRADE_FULLH1
let vect_steps = ref 0

let incr_vect_steps () = 
  incr vect_steps; 
  if !vect_steps > !Magic.vectsteps_limit then 
    raise OutOfTime 
  else if !vect_steps mod 10000 = 0 then 
    info (Printf.sprintf "vectsteps = %d" !vect_steps) 
  else
    ()

(*****************************************************************************)

open NonDetMonad

(* functions for accessing the monadic state *)
let fetchVarInfoM	 = fetchStateM >>= fun s -> unitM (get1of6 s) 
let fetchVisitedM	 = fetchStateM >>= fun s -> unitM (get2of6 s)
let fetchFpFusesM	 = fetchStateM >>= fun s -> unitM (get3of6 s)
let fetchSimdInstrsM	 = fetchStateM >>= fun s -> unitM (get4of6 s)
let fetchIdxInfoM	 = fetchStateM >>= fun s -> unitM (get5of6 s)
let fetchDepthsM	 = fetchStateM >>= fun s -> unitM (get6of6 s)
 
let storeVisitedM x	 = fetchStateM >>= fun s -> storeStateM (repl2of6 x s)
let storeFpFusesM x	 = fetchStateM >>= fun s -> storeStateM (repl3of6 x s)
let storeSimdInstrsM x   = fetchStateM >>= fun s -> storeStateM (repl4of6 x s)


let makeNewVSimdRegM () = unitM (makeNewVsimdreg ())

let addToVisitedM id = 
  fetchVisitedM >>= fun ids ->
    negAssertM (IdSet.mem id ids) >>
      storeVisitedM (IdSet.add id ids)

let addSimdInstrM i = fetchSimdInstrsM >>= consM i >>= storeSimdInstrsM 

let addSimdBinOpM op d (s1,s2) = addSimdInstrM (V_SimdBinOp(op,s1,s2,d))

let addSimdPNAccM d (s,t) =
  makeNewVSimdRegM () >>= fun d' ->
    addSimdInstrM (V_SimdBinOp(V_NPAcc,t,s,d')) >>
    addSimdInstrM (V_SimdUnaryOp(V_Swap,d',d)) 

let addSimdChsLoSubM d (s,t) =			   (* used with add/sub *)
  makeNewVSimdRegM () >>= fun t' ->
    addSimdInstrM (V_SimdUnaryOp(V_Chs V_Lo,t,t')) >>
    addSimdInstrM (V_SimdBinOp(V_Sub,s,t',d))

let addSimdChsHiAddM d (s,t) =			 (* used with x add/sub *)
  makeNewVSimdRegM () >>= fun s' ->
    addSimdInstrM (V_SimdUnaryOp(V_Chs V_Hi,s,s')) >>
    addSimdInstrM (V_SimdBinOp(V_Add,s',t,d))

let addSimdChsLoAddM d (s,t) =		(* used with sub/add, x sub/add *)
  makeNewVSimdRegM () >>= fun t' ->
    addSimdInstrM (V_SimdUnaryOp(V_Chs V_Lo,t,t')) >>
    addSimdInstrM (V_SimdBinOp(V_Add,s,t',d))

let addSimdSubChsHiM d (s,t) =			 (* used with x sub/sub *)
  makeNewVSimdRegM () >>= fun d' ->
    addSimdInstrM (V_SimdBinOp(V_Sub,s,t,d')) >>
    addSimdInstrM (V_SimdUnaryOp(V_Chs V_Hi,d',d))

(*****************************************************************************)

let fullvectgradeAllows = function
  | V_FPStore _, V_FPStore _ -> true
  | V_FPLoad _, V_FPLoad _ -> true
  | V_FPUnaryOp(op1,s1,_), V_FPUnaryOp(op2,s2,_) -> s1<>s2
  | V_FPBinOp(V_FPMul,_,_,_), V_FPBinOp(V_FPMul,_,_,_) -> true
  | V_FPBinOp(op1,_,_,_), V_FPBinOp(op2,_,_,_) ->
      vfpbinopIsAddorsub op1 && vfpbinopIsAddorsub op2
  | V_FPAddL(xs,_), V_FPAddL(zs,_) -> same_length xs zs && length xs > 2
  | _ -> false

let fullvectgradeh0Allows = function
  | V_FPStore(_,access1,_,i1), V_FPStore(_,access2,_,i2) -> 
      (match (access1,access2) with
	 | (V_FPReal, V_FPReal) -> abs(i1-i2) = !transformlength/2
	 | (V_FPRealOfComplex,V_FPRealOfComplex) ->
	     i1=0 && i2 = !transformlength/2 || i2=0 && i1 = !transformlength/2
	 | (V_FPRealOfComplex,V_FPImagOfComplex)
	 | (V_FPImagOfComplex,V_FPRealOfComplex) -> 
	     i1<>0 && i2<>0 && 
	     (abs(i1-i2) = !transformlength/4 || 
	      i1 = !transformlength/4 && i2 = !transformlength/4)
	 | _ -> false)
  | V_FPStore _, _ -> false
  | _, V_FPStore _ -> false
  | V_FPLoad(access1,_,i1,_), V_FPLoad(access2,_,i2,_) ->
      (match (access1,access2) with 
	 | (V_FPReal, V_FPReal) -> abs(i1-i2) = !transformlength/2
	 | (V_FPRealOfComplex, V_FPRealOfComplex) -> true
	 | (V_FPRealOfComplex, V_FPImagOfComplex) -> true
	 | (V_FPImagOfComplex, V_FPRealOfComplex) -> true
	 | (V_FPImagOfComplex, V_FPImagOfComplex) -> true
	 | _ -> false)
  | V_FPLoad _, _ -> false
  | _, V_FPLoad _ -> false
  | pair -> fullvectgradeAllows pair

let fullvectgradeh1Allows = function
  | V_FPStore(_,access1,_,i1), V_FPStore(_,access2,_,i2) -> 
      (match (access1,access2) with
	 | (V_FPReal, V_FPReal) -> abs(i1-i2) = !transformlength/2
	 | (V_FPRealOfComplex,V_FPRealOfComplex) ->
	     i1=0 && i2 = !transformlength/2 || i2=0 && i1 = !transformlength/2
	 | (V_FPRealOfComplex,V_FPImagOfComplex)
	 | (V_FPImagOfComplex,V_FPRealOfComplex) -> i1<>0 && i2<>0 && i1=i2
	 | _ -> false)
  | V_FPStore _, _ -> false
  | _, V_FPStore _ -> false
  | V_FPLoad(access1,_,i1,_), V_FPLoad(access2,_,i2,_) ->
      (match (access1,access2) with 
	 | (V_FPReal, V_FPReal) -> abs(i1-i2) = !transformlength/2
	 | (V_FPRealOfComplex, V_FPRealOfComplex) -> true
	 | (V_FPRealOfComplex, V_FPImagOfComplex) -> true
	 | (V_FPImagOfComplex, V_FPRealOfComplex) -> true
	 | (V_FPImagOfComplex, V_FPImagOfComplex) -> true
	 | _ -> false)
  | V_FPLoad _, _ -> false
  | _, V_FPLoad _ -> false
  | pair -> fullvectgradeAllows pair

let fullvectgradeh2Allows = function
  | V_FPStore(_,access1,_,i1), V_FPStore(_,access2,_,i2) ->
      (match (access1,access2) with
	 | (V_FPRealOfComplex, V_FPImagOfComplex) 
	 | (V_FPImagOfComplex, V_FPRealOfComplex) -> i1+i2 = !transformlength/2
	 | (V_FPReal, V_FPReal) -> abs(i1-i2) = !transformlength/4
	 | _ -> true)
  | V_FPStore _, _ -> false
  | _, V_FPStore _ -> false
  | V_FPLoad(access1,_,i1,_), V_FPLoad(access2,_,i2,_) ->
      (match (access1,access2) with
	 | (V_FPReal, V_FPReal) -> abs(i1-i2) = !transformlength/2 
	 | _ -> true)
  | V_FPLoad _, _ -> false
  | _, V_FPLoad _ -> false
  | pair -> fullvectgradeAllows pair

(****************************************************************************)

let semivectgradeAllows = function
  | V_FPStore _, V_FPStore _ -> true
  | V_FPLoad _, V_FPLoad _ -> true
  | V_FPLoad _, V_FPBinOp(op,_,_,_) -> vfpbinopIsAddorsub op
  | V_FPBinOp(op,_,_,_), V_FPLoad _ -> vfpbinopIsAddorsub op
  | V_FPUnaryOp(_,s1,_), V_FPUnaryOp(_,s2,_) -> s1<>s2 
  | _, V_FPUnaryOp _ -> true
  | V_FPUnaryOp _, _ -> true
  | V_FPBinOp(V_FPMul,_,_,_), V_FPBinOp(V_FPMul,_,_,_) -> true
  | V_FPBinOp(op1,_,_,_), V_FPBinOp(op2,_,_,_) ->
      vfpbinopIsAddorsub op1 && vfpbinopIsAddorsub op2
  | V_FPAddL(xs,_), V_FPAddL(zs,_) -> same_length xs zs && length xs > 2
  | _ -> false

let semivectgradeh1Allows = function
  | V_FPStore(_,access1,_,i1), V_FPStore(_,access2,_,i2) -> 
      (match (access1,access2) with
	 | (V_FPReal, V_FPReal) -> abs(i1-i2) = !transformlength/2
	 | (V_FPRealOfComplex,V_FPImagOfComplex) -> i1<>0 && i1=i2 
	 | (V_FPImagOfComplex,V_FPRealOfComplex) -> i2<>0 && i1=i2
	 | (V_FPRealOfComplex,V_FPRealOfComplex) ->
	     i1=0 && i2 = !transformlength/2 || i2=0 && i1 = !transformlength/2
	 | _ -> false)
  | pair -> semivectgradeAllows pair

(****************************************************************************)

let currentvectgradeAllowsPairing pair = 
  match !current_vectorization_grade with
    | VECTGRADE_FULLH0 -> fullvectgradeh0Allows pair
    | VECTGRADE_FULLH1 -> fullvectgradeh1Allows pair
    | VECTGRADE_FULLH2 -> fullvectgradeh2Allows pair
    | VECTGRADE_FULL   -> fullvectgradeAllows pair
    | VECTGRADE_SEMIH1 -> semivectgradeh1Allows pair
    | VECTGRADE_SEMI   -> semivectgradeAllows pair
  
let currentvectgradeAllowsIds instr_depths (id_a,id_b) =
  match !current_vectorization_grade with
    | VECTGRADE_FULLH0 | VECTGRADE_FULLH1 
    | VECTGRADE_FULLH2 | VECTGRADE_FULL -> 
	IdMap.find id_a instr_depths = IdMap.find id_b instr_depths
    | VECTGRADE_SEMIH1 | VECTGRADE_SEMI -> 
	true

let basicallyCompatibleM ((id_a,a),(id_b,b)) = 
  posAssertM (currentvectgradeAllowsPairing (a,b)) >>
    fetchDepthsM >>= fun depths ->
      posAssertM (currentvectgradeAllowsIds depths (id_a,id_b))

(****************************************************************************)

let compatibleFusesM mode = function
  | ReImSimd(x,y,z), ImReSimd(y',x',z') when x=x' && y=y' && z=z' ->
      unitM z
  | ImReSimd(y,x,z), ReImSimd(x',y',z') when x=x' && y=y' && z=z' ->
      makeNewVSimdRegM () >>= fun z' ->
        (match mode with
	   | FuseModeSrc -> addSimdInstrM (V_SimdUnaryOp(V_Swap,z,z'))
	   | FuseModeDst -> addSimdInstrM (V_SimdUnaryOp(V_Swap,z',z))) 
  	>>= fun _ ->
	  unitM z'
  | _ ->
      failM

let compatibleVisitedForDualVectM s1_id s2_id = 
  if s1_id=s2_id then
    failM 
  else
    fetchVisitedM >>= fun visited ->
      posAssertM (IdSet.mem s1_id visited = IdSet.mem s2_id visited)

(****************************************************************************)

let getUnvisitedLoadM () = 
  fetchIdxInfoM >>= fun (loads,_,_) -> 
    memberM loads >>= fun ((id,_) as pair) -> 
      addToVisitedM id >> 
	unitM pair

let getUnvisitedStoreM () = 
  fetchIdxInfoM >>= fun (_,stores,_) ->
    memberM stores >>= fun ((id,_) as pair) -> 
      addToVisitedM id >> 
	unitM pair

(****************************************************************************)

let scalaropToAccM = function
  | V_FPAdd -> unitM V_PPAcc
  | V_FPSub -> unitM V_NNAcc
  | _ -> failwith "scalaropToAccM"

let unaryoppairToSimdunaryopM = function 
  | (V_FPId, op_h) -> unitM (scalarvfpunaryopToVsimdunaryop op_h V_Hi)
  | (op_l, V_FPId) -> unitM (scalarvfpunaryopToVsimdunaryop op_l V_Lo)
  | (V_FPNegate, V_FPNegate) -> unitM (V_Chs(V_LoHi))
  | (V_FPNegate, V_FPMulConst h) -> unitM (V_MulConst(Number.mone,h))
  | (V_FPMulConst l, V_FPNegate) -> unitM (V_MulConst(l,Number.mone))
  | (V_FPMulConst l, V_FPMulConst h) -> unitM (V_MulConst(l,h))

(****************************************************************************)

let srcsToParpair (s1,t1,s2,t2) = ((s1,s2), (t1,t2))
let srcsToChipair (s1,t1,s2,t2) = ((s1,t2), (t1,s2))
let srcsToAccpair (s1,t1,s2,t2) = ((s1,t1), (s2,t2))


let addNewFpFuseM re im simd =
  fetchFpFusesM >>= fun fp_fuses' ->
    let fp_fuses'' = vfpregmap_addE re (ReImSimd(re,im,simd)) fp_fuses' in
      storeFpFusesM (vfpregmap_addE im (ImReSimd(im,re,simd)) fp_fuses'')

let addFpFuseM s1 s2 =
  posAssertM (s1<>s2) >>= fun _ ->
    fetchIdxInfoM >>= fun (_,_,writers) ->
      let ((s1_id,_) as s1_writer) = VFPRegMap.find s1 writers 
      and ((s2_id,_) as s2_writer) = VFPRegMap.find s2 writers in
	compatibleVisitedForDualVectM s1_id s2_id >>
	  basicallyCompatibleM (s1_writer,s2_writer) >>
            makeNewVSimdRegM () >>= fun simd ->
	      addNewFpFuseM s1 s2 simd >>
	        unitM (simd,[DualVect(simd,s1_writer,s2_writer)])

let fuseNewM () = 
  let (t_re,t_im) = (makeNewVfpreg (), makeNewVfpreg ()) in
    makeNewVSimdRegM () >>= fun t_simd ->
      addNewFpFuseM t_re t_im t_simd >>
	unitM (t_simd,t_re,t_im)

let fuseM mode = function
  | (re,im) when re=im -> failM
  | (re,im) ->
      fetchFpFusesM >>= fun map ->
	match (vfpregmap_findE re map, vfpregmap_findE im map) with
	    | [],[] -> addFpFuseM re im
	    | (_::_ as xs), (_::_ as zs) ->
		 memberM xs >>= fun x ->
		   memberM zs >>= fun z ->
		     compatibleFusesM mode (x,z) >>= fun simd ->
			unitM (simd,[])
	    | _ -> failM

let fuseSrcM p = fuseM FuseModeSrc p
let fuseDstM p = fuseM FuseModeDst p >>= fun (d_simd,_) -> unitM d_simd

(*****************************************************************************)

let fuseSrcPair_ParM srcs = mapPairM fuseSrcM (srcsToParpair srcs)
let fuseSrcPair_ChiM srcs = mapPairM fuseSrcM (srcsToChipair srcs)
let fuseSrcPair_AccM srcs = mapPairM fuseSrcM (srcsToAccpair srcs)

(*****************************************************************************)

let vectFp_AddAdd_Fns = 
	[
	 (fuseSrcPair_AccM, addSimdBinOpM V_PPAcc);		(* PosPosAcc *)
	 (fuseSrcPair_ParM, addSimdBinOpM V_Add);		(* ParAddAdd *)
	 (fuseSrcPair_ChiM, addSimdBinOpM V_Add);		(* ChiAddAdd *)
	]

let vectFp_AddSub_Fns = 
	[
	 (fuseSrcPair_AccM, addSimdPNAccM);			(* PosNegAcc *)
	 (fuseSrcPair_ParM, addSimdChsLoSubM);			(* ParAddSub *)
	 (fuseSrcPair_ChiM, addSimdChsHiAddM);			(* ChiAddSub *)
	]

let vectFp_SubAdd_Fns =
	[
	 (fuseSrcPair_AccM, addSimdBinOpM V_NPAcc);		(* NegPosAcc *)
	 (fuseSrcPair_ParM, addSimdChsLoAddM); 			(* ParSubAdd *)
	 (fuseSrcPair_ChiM, addSimdChsLoAddM);			(* ChiSubAdd *)
	]

let vectFp_SubSub_Fns =
	[
	 (fuseSrcPair_AccM, addSimdBinOpM V_NNAcc); 		(* NegNegAcc *)
	 (fuseSrcPair_ParM, addSimdBinOpM V_Sub);		(* ParSubSub *)
	 (fuseSrcPair_ChiM, addSimdSubChsHiM);			(* ChiSubSub *)
	]

let vectFp_MulMul_Fns =
	[
	 (fuseSrcPair_ParM, addSimdBinOpM V_Mul);		(* ParMulMul *)
	 (fuseSrcPair_ChiM, addSimdBinOpM V_Mul);		(* ChiMulMul *)
	]

let binoppairToVectfnM = function
  | (V_FPMul, V_FPMul) -> memberM vectFp_MulMul_Fns
  | (V_FPAdd, V_FPAdd) -> memberM vectFp_AddAdd_Fns
  | (V_FPAdd, V_FPSub) -> memberM vectFp_AddSub_Fns
  | (V_FPSub, V_FPAdd) -> memberM vectFp_SubAdd_Fns
  | (V_FPSub, V_FPSub) -> memberM vectFp_SubSub_Fns
  | _ -> failM

(****************************************************************************)

let vectorizeLoadM (s1_access,s1_arr,s1_idx,d1) (s2_access,s2_arr,s2_idx,d2) = 
  fetchVarInfoM >>= fun varinfo ->
    match (assoc s1_arr varinfo, assoc s2_arr varinfo) with
      | (ComplexArray(InterleavedFormat), ComplexArray(InterleavedFormat)) ->
	  posAssertM (s1_arr=s2_arr && s1_idx=s2_idx) >>
	    (match (s1_access,s2_access) with
	       | (V_FPRealOfComplex,V_FPImagOfComplex) -> unitM (d1,d2)
	       | (V_FPImagOfComplex,V_FPRealOfComplex) -> unitM (d2,d1)
	       | _ -> failM) 
	    >>= fun fuse_pair ->
	      fuseDstM fuse_pair >>= fun d_simd ->
	        addSimdInstrM (V_SimdLoadQ(s1_arr,s1_idx,d_simd)) >>
	          unitM []
      | (ComplexArray(InterleavedFormat), _) -> failM
      | (_, ComplexArray(InterleavedFormat)) -> failM
      | _ ->
	  fuseDstM (d1,d2) >>= fun dst'' ->
	    makeNewVSimdRegM () >>= fun dst ->
	    makeNewVSimdRegM () >>= fun dst' ->
	      addSimdInstrM (V_SimdLoadD(s1_access,s1_arr,s1_idx,V_Lo,dst)) >>
	      addSimdInstrM (V_SimdLoadD(s2_access,s2_arr,s2_idx,V_Lo,dst')) >>
	      addSimdInstrM (V_SimdBinOp(V_UnpckLo,dst,dst',dst'')) >>
	        unitM []

(****************************************************************************)

let vectorizeStoreM (s1,d1_access,d1_arr,d1_idx) (s2,d2_access,d2_arr,d2_idx) =
  fetchVarInfoM >>= fun varinfo ->
    match (assoc d1_arr varinfo, assoc d2_arr varinfo) with
      | (ComplexArray(InterleavedFormat), ComplexArray(InterleavedFormat)) ->
  	  posAssertM (d1_arr=d2_arr && d1_idx=d2_idx) >>
	    (match (d1_access,d2_access) with
	       | (V_FPRealOfComplex,V_FPImagOfComplex) -> unitM (s1,s2)
	       | (V_FPImagOfComplex,V_FPRealOfComplex) -> unitM (s2,s1)
	       | _ -> failM) 
	    >>= fun fuse_pair ->
	      fuseSrcM fuse_pair >>= fun (s_simd,dvects) ->
	        addSimdInstrM (V_SimdStoreQ(s_simd,d1_arr,d1_idx)) >>
		  unitM dvects
      | (_,ComplexArray(InterleavedFormat)) -> failM
      | (ComplexArray(InterleavedFormat),_) -> failM
      | _ ->
	  fuseSrcM (s1,s2) >>= fun (s,dvects) ->
	    makeNewVSimdRegM () >>= fun s' ->
	      addSimdInstrM (V_SimdStoreD(V_Lo,s,d1_access,d1_arr,d1_idx)) >>
	      addSimdInstrM (V_SimdBinOp(V_UnpckHi,s,s,s')) >>
	      addSimdInstrM (V_SimdStoreD(V_Lo,s',d2_access,d2_arr,d2_idx)) >>
		unitM dvects

(****************************************************************************)

let rec vectorizeScalarInstr2M' d_simd ids instrs =
  vectorizeScalarInstr2M d_simd ids instrs >>= fun (dvects, new_visited) ->
    iterM addToVisitedM new_visited >>
      unitM dvects

and vectorizeScalarInstr2M d_simd (re_id,im_id) = function
  | V_FPLoad(s_access,s_arr,s_idx,d1), V_FPBinOp(op,s1,s2,d2) ->
      scalaropToAccM op >>= fun op' ->
	fuseSrcM (s1,s2) >>= fun (s_simd,dvects) ->
	  makeNewVSimdRegM () >>= fun t_simd ->
	    addSimdInstrM (V_SimdLoadD(s_access,s_arr,s_idx,V_Lo,t_simd)) >>
	    addSimdInstrM (V_SimdBinOp(op', t_simd, s_simd, d_simd)) >>
	      unitM (dvects, [re_id; im_id])

  | V_FPBinOp(op,s1,s2,d1), V_FPLoad(s_access,s_arr,s_idx,d2) ->
      scalaropToAccM op >>= fun op' ->
	fuseSrcM (s1,s2) >>= fun (s_simd,dvects) ->
	  makeNewVSimdRegM () >>= fun t_simd ->
	    addSimdInstrM (V_SimdLoadD(s_access,s_arr,s_idx,V_Lo,t_simd)) >>
	    addSimdInstrM (V_SimdBinOp(op', s_simd, t_simd, d_simd)) >>
	      unitM (dvects, [re_id; im_id])

  | V_FPUnaryOp(op1,s1,d1), V_FPUnaryOp(op2,s2,d2) -> 
      unaryoppairToSimdunaryopM (op1,op2) >>= fun simd_op ->
        fuseSrcM (s1,s2) >>= fun (s_simd,dvects) ->
          addSimdInstrM (V_SimdUnaryOp(simd_op,s_simd,d_simd)) >>
	    unitM (dvects, [re_id; im_id])

  | instrL, V_FPUnaryOp(op,s2,d2) ->
      let s1  = optionToValue (vfpinstrToDstreg instrL)
      and op' = scalarvfpunaryopToVsimdunaryop op V_Hi in
        addFpFuseM s1 s2 >>= fun (d_simd',dvects) -> 
	  addSimdInstrM (V_SimdUnaryOp(op', d_simd', d_simd)) >>
	    unitM (dvects, [im_id])

  | V_FPUnaryOp(op,s1,d1), instrR ->
      let s2  = optionToValue (vfpinstrToDstreg instrR)
      and op' = scalarvfpunaryopToVsimdunaryop op V_Lo in
	addFpFuseM s1 s2 >>= fun (d_simd',dvects) -> 
	  addSimdInstrM (V_SimdUnaryOp(op', d_simd', d_simd)) >>
	    unitM (dvects, [re_id])

  | V_FPBinOp(op1,s1,t1,d1), V_FPBinOp(op2,s2,t2,d2) ->
      let src_info = (s1,t1,s2,t2) in
        binoppairToVectfnM (op1,op2) >>= fun (fuse_fn,addinstr_fn) ->
	  fuse_fn src_info >>= fun ((x_simd,x_instrs), (y_simd,y_instrs)) ->
	    addinstr_fn d_simd (x_simd, y_simd) >>
	      unitM (x_instrs @ y_instrs, [re_id; im_id])

  | V_FPAddL(s1,d1), V_FPAddL(s2,d2) ->
      iterM addToVisitedM [re_id; im_id] >> (* earlier is better? *)
	(if length s2 < 3 then unitM s2 else permutationM s2) >>= fun s2p -> 
	  vectorizeAddListM (d_simd,d1,d2) (s1,s2p) >>= fun (dvs,s1',s2') ->
	    if vfpsummandIsPositive s1' && vfpsummandIsPositive s2' then
	      unitM (dvs, [])
	    else
	      failwith "vectorizeScalarInstr2M: inconsistent addlist-result"

  | (V_FPLoad(s1_access,s1_arr,s1_idx,d1), 
     V_FPLoad(s2_access,s2_arr,s2_idx,d2)) ->
      vectorizeLoadM (s1_access,s1_arr,s1_idx,d1)
		     (s2_access,s2_arr,s2_idx,d2) >>= fun dvects ->
        unitM (dvects, [re_id; im_id])

  | _ ->
      failM

(****************************************************************************)

(* [precondition:] List.length xs = List.length zs *)
and vectorizeAddListM ((d_simd,d_re,d_im) as d) = function 
  | [], [] -> failwith "vectorizeAddListM: listlength = 0 not supported!"
  | [x'], [z'] -> 
	unitM ([],x', z')
  | [x1';x2'], [z1';z2'] ->
      let (i1,d_re') = addlistHelper2 d_re (x1',x2') 
      and (i2,d_im') = addlistHelper2 d_im (z1',z2') in
	let id_1 = Id.makeNew ()		(* this sucks *)
	and id_2 = Id.makeNew () in
        vectorizeScalarInstr2M' d_simd (id_1,id_2) (i1,i2) >>= fun dvects ->
	  unitM (dvects,d_re', d_im')
  | xs, zs ->
      let xs_l, xs_r = zip xs in
      let zs_l, zs_r = zip zs in
	fuseNewM () >>= fun ((d_l_simd,d_l_re,d_l_im) as d_l) ->
	fuseNewM () >>= fun ((d_r_simd,d_r_re,d_r_im) as d_r) ->
          vectorizeAddListM d_l (xs_l,zs_l) >>= fun (dvs1,d_l_re', d_l_im') ->
	  vectorizeAddListM d_r (xs_r,zs_r) >>= fun (dvs2,d_r_re', d_r_im') ->
	    vectorizeAddListM d ([d_l_re'; d_r_re'], [d_l_im'; d_r_im']) 
	    >>= fun (dvs3, ret1, ret2) ->
	      unitM (dvs1@dvs2@dvs3, ret1, ret2)

(****************************************************************************)

and vectorizeScalarInstrM me_id here_instr = match here_instr with
  | V_FPStore _ as this_store ->
      addToVisitedM me_id >>
	getUnvisitedStoreM () >>= fun (other_id,other_store) ->
	  posAssertM (currentvectgradeAllowsPairing 
				(this_store,other_store)) >>= fun _ ->
	  (match (this_store,other_store) with
	     | (V_FPStore(s1,d1_access,d1_arr,d1_idx), 
	        V_FPStore(s2,d2_access,d2_arr,d2_idx)) ->
		  vectorizeStoreM (s1,d1_access,d1_arr,d1_idx)
				  (s2,d2_access,d2_arr,d2_idx)

	     | _ -> failwith "fpstore inconsistent!")

  | V_FPLoad _ as this_load ->
      addToVisitedM me_id >>
	getUnvisitedLoadM () >>= fun (other_id,other_load) ->
	  posAssertM (currentvectgradeAllowsPairing 
			(this_load,other_load)) >>= fun _ ->
	    (match (this_load,other_load) with
	       | (V_FPLoad(s1_access,s1_arr,s1_idx,d1), 
		  V_FPLoad(s2_access,s2_arr,s2_idx,d2)) ->
	            vectorizeLoadM (s1_access,s1_arr,s1_idx,d1)
			           (s2_access,s2_arr,s2_idx,d2) >> 
		      unitM []
	       | _ -> failwith "fpload inconsistent!")

  | instr ->
      failwith "vectorizeScalarInstrM: SingleVect(Any) is not supported!"

and vectorize2M = function
  | SingleVect((i_id,i_instr)) ->
      fetchVisitedM >>= fun visited ->
        if IdSet.mem i_id visited then
          unitM []
        else
          vectorizeScalarInstrM i_id i_instr
  | DualVect(d_simd,((i1_id,i1_instr) as i1_info),
		    ((i2_id,i2_instr) as i2_info)) ->
      fetchVisitedM >>= fun visited ->
	(match (IdSet.mem i1_id visited, IdSet.mem i2_id visited) with
	   | (true, true) -> unitM []
	   | (false, false) ->
	       vectorizeScalarInstr2M 
			d_simd 
			(i1_id,i2_id) 
			(i1_instr,i2_instr) 
	       >>= fun (dvects, newvisited) ->
		 iterM addToVisitedM newvisited >>
		   unitM dvects
	   | (false,true)
	   | (true, false) ->
	       failM)

and vectorizeM = function
  | [] -> unitM () 
  | x::xs ->
      incr_vect_steps ();
	vectorize2M x >>= 
	(* In some cases it may be favorable to reorder instructions, ie.,
	 * to delay the vectorization of expensive instruction (addList).
 	 *
	 * This is *not* done at the moment, however. 
	 *)
	  vectorizeM >> 
	    vectorizeM xs

(****************************************************************************)

let arrayHasInterleavedcomplexformat arr info = match listAssoc arr info with
  | Some(ComplexArray(InterleavedFormat)) -> true
  | _ -> false

let checkVectgradeRequirementsM all_instrs grade =
  posAssertM (count vfpinstrIsStore all_instrs mod 2 = 0) >>
    (match grade with
      | VECTGRADE_SEMIH1 
      | VECTGRADE_FULLH0 | VECTGRADE_FULLH1 | VECTGRADE_FULLH2 ->
	  fetchVarInfoM >>= fun varinfo ->
	    negAssertM (arrayHasInterleavedcomplexformat Input varinfo) >>
	    negAssertM (arrayHasInterleavedcomplexformat Twiddle varinfo)
      | _ ->
	  unitM ()) >>
    (match grade with
      | VECTGRADE_FULL | VECTGRADE_FULLH0 
      | VECTGRADE_FULLH1 | VECTGRADE_FULLH2 ->
	  let nr_addsubs = sum_list (map vfpinstrToAddsubcount all_instrs)
	  and nr_muls = count vfpinstrIsBinMul all_instrs 
	  and nr_unaryops = count vfpinstrIsUnaryOp all_instrs 
	  and nr_loads = count vfpinstrIsLoad all_instrs in
	    posAssertM (nr_addsubs mod 2 = 0 &&
			nr_muls mod 2 = 0 &&
			nr_unaryops mod 2 = 0 &&
			nr_loads mod 2 = 0)
      | _ ->
	  unitM ())


let vectorizeGradedM all_instrs xs = 
  memberM vectorization_grades >>= fun grade ->
    checkVectgradeRequirementsM all_instrs grade >>= fun _ ->
      let _ = 
	begin
	  info (Printf.sprintf "trying grade: %s" 
			       (vectorizationgradeToString grade));
	  current_vectorization_grade := grade;
          vect_steps := 0
	end in
          catch1M 
		OutOfTime 
		(fun _ -> info "vectorization terminated (TIMEOUT)"; failM) >>
	    vectorizeM xs >>
	      fetchSimdInstrsM >>= fun vectorized_instrs' ->
	        unitM (K7_QWord, vectorized_instrs')
  
(****************************************************************************)

let rec calc_min_depths depthmap writers xs =
  List.fold_right (fun i s0 -> calc_min_depth s0 writers i) xs depthmap

and calc_min_depth depthmap writers (id,instr) =
  if idmap_exists id depthmap then
    depthmap
  else
    calc_min_depth' depthmap writers (id,instr)

and calc_min_depth' depthmap writers (id,instr) =
  let srcs = vfpinstrToSrcregs instr in
  let srcs_writers = List.map (fun src -> VFPRegMap.find src writers) srcs in
  let depthmap' = calc_min_depths depthmap writers srcs_writers in
  let vals = List.map (fun (id,_) -> IdMap.find id depthmap') srcs_writers in
  let minval = if vals = [] then 0 else min_list vals in
    IdMap.add id (succ minval) depthmap'

(***************************************************************************)

let vectorize' varinfo n vinstrs =
  let vinstrs' = map (fun i -> (Id.makeNew (),i)) vinstrs in
  let (loads,other') = partition (fun (_,i) -> vfpinstrIsLoad i) vinstrs' in
  let (stores,_) = partition (fun (_,i) -> vfpinstrIsStore i) other' in
  let writers = 
	fold_right
		(fun ((i_id,i_instr) as i) ->
		   match vfpinstrToDstreg i_instr with
		     | None   -> return
		     | Some d -> VFPRegMap.add d i)
		vinstrs'
		VFPRegMap.empty in
  let depths = calc_min_depths IdMap.empty writers stores in
    runM (vectorizeGradedM vinstrs) 
	 (map (fun p -> SingleVect p) stores) 
         (varinfo,IdSet.empty,VFPRegMap.empty,[],(loads,stores,writers),depths)

(***************************************************************************)

let vectorize varinfo n vinstrs =
  info "vectorize: trying cheap vectorization";
  transformlength := n;
  let vinstrs' = concat (map fullyexpandaddlistVfpinstr vinstrs) in
    match vectorize' varinfo n vinstrs' with
      | Some instrs -> 
	  Printf.printf "/* cheap-mode: %s succeeded. (%d steps) */\n"
			(vectorizationgradeToString 
				!current_vectorization_grade)
			!vect_steps;
	  instrs
      | None ->
	  info "vectorize: trying $$$ vectorization";
	  match vectorize' varinfo n vinstrs with
	    | Some instrs -> 
		Printf.printf 
			"/* $$$-mode: %s succeeded (%d steps) */\n"
			(vectorizationgradeToString 
				!current_vectorization_grade)
			!vect_steps;
		instrs
	    | None -> 
		print_string
			"/* using vectorization-grade: VECTGRADE_NULL */\n";
		NullVectorization.nullVectorize n vinstrs
