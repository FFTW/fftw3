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
open Expr
open Number
open Variable
open VFpBasics
open VSimdBasics
open StateMonad


(* monadic access functions *)
let fetchArrayInfoM = fetchStateM >>= fun s -> unitM (fst s)
let fetchVfpinstrsM = fetchStateM >>= fun s -> unitM (snd s)

let storeVfpinstrsM x = fetchStateM >>= fun s -> storeStateM (repl2of2 x s)

(****************************************************************************)

let addVfpinstrM i = fetchVfpinstrsM >>= consM i >>= storeVfpinstrsM

let arrayToTypeM key = fetchArrayInfoM >>= fun map -> unitM (assoc key map)

(****************************************************************************)

let arrayinfoToAccess = function
  | (ComplexArray _, RealPart) -> V_FPRealOfComplex
  | (ComplexArray _, ImagPart) -> V_FPImagOfComplex
  | (RealArray, RealPart)      -> V_FPReal
  | (RealArray, ImagPart)      -> failwith "arrayinfoToAccess"

let variableToTupledForm = function
  | RealArrayElem(arr,idx) -> (RealPart,arr,idx)
  | ImagArrayElem(arr,idx) -> (ImagPart,arr,idx)
  | _ 			   -> failwith "variableToTupledForm"

(****************************************************************************)

let varinfo_notwiddle = 
	[(Input,   ComplexArray(InterleavedFormat)); 
	 (Output,  ComplexArray(InterleavedFormat))]

let varinfo_twiddle = 
	[(Input,   ComplexArray(InterleavedFormat)); 
	 (Twiddle, ComplexArray(InterleavedFormat)); 
	 (Output,  ComplexArray(InterleavedFormat))]

let varinfo_real2hc = 
	[(Input,  RealArray); 
	 (Output, ComplexArray(SplitFormat))]

let varinfo_hc2real = 
	[(Input,  ComplexArray(SplitFormat));
	 (Output, RealArray)]

let varinfo_hc2hc_zerothelements =
	[(Input,   ComplexArray(SplitFormat)); 
	 (Output,  ComplexArray(SplitFormat))]

let varinfo_hc2hc_finalelements = varinfo_hc2hc_zerothelements

let varinfo_hc2hc_middleelements =
	[(Input,   ComplexArray(SplitFormat)); 
	 (Output,  ComplexArray(SplitFormat)); 
	 (Twiddle, ComplexArray(InterleavedFormat))]

let varinfo_realeven = 
	[(Input,  RealArray); 
	 (Output, RealArray)]

let varinfo_realodd = 
	[(Input,  ComplexArray(SplitFormat));
	 (Output, ComplexArray(SplitFormat))]

(****************************************************************************)

let conditionallyCopyVarM s = function
  | Some d when s <> d ->
      failwith (Printf.sprintf "conditional_copy_var: unsupported! %s %s" 
		  (VFpUnparsing.vfpregToString s) 
		  (VFpUnparsing.vfpregToString d))
  | _ -> unitM s

let extractUseReturnVfpregM fM = function
  | Some x -> fM x >> unitM x
  | None   -> let x = makeNewVfpreg () in fM x >> unitM x

let extractUseVfpregM fM x = extractUseReturnVfpregM fM x >> unitM ()

let varexprToVfpreg = function
  | Temporary s -> V_FPReg s
  | _	 	-> failwith "varexprToVfpreg"

let timesexprToNormalized = function
  | (Num _, Num _)	-> failwith "timesexprToNormalized"
  | (Num _, _) as pair	-> pair
  | (x, (Num n as num))	-> (num,x)
  | pair		-> pair

let plusexprToChecked = function
  | []  -> failwith "plusexprToVfpinstrsM []"
  | [_] -> failwith "plusexprToVfpinstrsM [_]"
  | xs  -> xs 

let rec exprToNegated = function
  | Uminus x 	   -> x
  | Load x 	   -> Uminus (Load x)
  | Plus xs 	   -> Plus (map exprToNegated (plusexprToChecked xs))
  | Times(Num n,x) -> Times(Num (Number.negate n), x)
  | Times(x,Num n) -> Times(x, Num (Number.negate n))
  | Times(x,y) 	   -> Times(exprToNegated x, y) 
  | Num n 	   -> Num (Number.negate n)
  | Store(_,_)     -> failwith "exprToNegated"

(****************************************************************************)

let rec summandCmp a b = match (a,b) with
  | (Uminus x, y) -> summandCmp x y
  | (x, Uminus y) -> summandCmp x y
  | (Load x, Load y) -> compare x y
  | (Load _, Times _) -> -1
  | (Times _, Load _) -> 1
  | (Times(Num _, Num _), _) -> failwith "summandCmp: not supported (1)!"
  | (_, Times(Num _, Num _)) -> failwith "summandCmp: not supported (2)!"
  | (Times(x, Num n), y) -> summandCmp (Times(Num n, x)) y
  | (x, Times(y, Num n)) -> summandCmp x (Times(Num n, y))
  | (Times(Num _, x), Times(Num _, y)) -> compare x y
  | (t1,t2) -> compare t1 t2

(****************************************************************************)

let rec exprToVfpinstrsM expr = 
  exprToVfpinstrsM' expr None

and exprToVfpinstrsM' = function
  | Load (Temporary _ as x) -> conditionallyCopyVarM (varexprToVfpreg x)
  | Load x      -> extractUseReturnVfpregM (loadToVfpinstrsM x)
  | Times(x,y) -> timesexprToVfpinstrsM (timesexprToNormalized (x,y))
  | Plus xs    -> plusexprToVfpinstrsM (plusexprToChecked xs)
  | Uminus (Load v) -> extractUseReturnVfpregM (uminusvarexprToVfpinstrM v)
  | Uminus x   -> exprToVfpinstrsM' (exprToNegated x)
  | _      -> failwith "exprToVfpinstrsM: _ not supported!"

and uminusvarexprToVfpinstrM v dst' =
  addVfpinstrM (V_FPUnaryOp(V_FPNegate,varexprToVfpreg v,dst'))

and timesexprToVfpinstrsM = function
  | (Num n, x) -> extractUseReturnVfpregM (timesconstexprToVfpinstrsM' x n)
  | (x1,x2)    -> extractUseReturnVfpregM (timesexprToVfpinstrsM' x1 x2)
and timesconstexprToVfpinstrsM' x n dst' =
  exprToVfpinstrsM x >>= fun x' ->
    addVfpinstrM (V_FPUnaryOp(V_FPMulConst(n),x',dst'))
and timesexprToVfpinstrsM' x1 x2 dst' =
  mapPairM exprToVfpinstrsM (x1,x2) >>= fun (x1',x2') ->
    addVfpinstrM (V_FPBinOp(V_FPMul,x1',x2',dst'))

and plusexprToVfpinstrsM xs dst =
  mapM summandToVfpsummandM (sort summandCmp xs) >>= fun xs' ->
    extractUseReturnVfpregM (fun dst' -> addVfpinstrM (V_FPAddL(xs',dst'))) dst
and summandToVfpsummandM = function
  | Uminus x -> exprToVfpinstrsM x >>= fun dst' -> unitM (V_FPMinus dst')
  | x        -> exprToVfpinstrsM x >>= fun dst' -> unitM (V_FPPlus dst')

(****************************************************************************)

and loadToVfpinstrsM src dst' =
  let (place,arr,idx) = variableToTupledForm src in
    arrayToTypeM arr >>= fun arrtype ->
      addVfpinstrM (V_FPLoad(arrayinfoToAccess (arrtype,place),arr,idx,dst'))

and storeToVfpinstrsM dst src' =
  let (place,arr,idx) = variableToTupledForm dst in
    arrayToTypeM arr >>= fun arrtype ->
      addVfpinstrM (V_FPStore(src',arrayinfoToAccess (arrtype,place),arr,idx))

(****************************************************************************)

let assignmentToVfpinstrsM = function
  | Assign(v,Load e) when is_input e || is_twiddle e ->
      if is_output v then
	extractUseReturnVfpregM (loadToVfpinstrsM e) None >>= 
	  storeToVfpinstrsM v
      else
        extractUseVfpregM (loadToVfpinstrsM e) (Some (varexprToVfpreg v))
  | Assign(v,e) when is_output v -> 
      exprToVfpinstrsM e >>=
        storeToVfpinstrsM v 
  | Assign(Temporary d,e) -> 
      ignoreM (exprToVfpinstrsM' e (Some (V_FPReg d)))
  | _ -> 
      failwith "assignmentToVfpinstrsM"

let assignmentsToVfpinstrsM xs =
  iterM assignmentToVfpinstrsM xs >>
    fetchVfpinstrsM >>= fun vfpinstrs_rev ->
      unitM (rev vfpinstrs_rev)

open BalanceVfpinstrs

let assignmentsToVfpinstrs arraytypeinfo xs = 
  let _ = Util.info "assignmentsToVfpinstrs (1)" in
  let i1 = StateMonad.runM assignmentsToVfpinstrsM xs (arraytypeinfo,[]) in
  let _ = Util.info "assignmentsToVfpinstrs (2)" in
  let i2 = balance [] i1 in
  let _ = Util.info "assignmentsToVfpinstrs (3)" in
    map expandaddlistVfpinstr i2



