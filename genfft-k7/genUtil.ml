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
open Variable
open Number
open VSimdBasics
open VSimdUnparsing
open VScheduler
open VAnnotatedScheduler
open K7Basics
open K7RegisterAllocationBasics
open K7RegisterReallocation
open K7Unparsing
open K7Translate
open K7FlatInstructionScheduling
open K7InstructionSchedulingBasics
open Printf
open AssignmentsToVfpinstrs
open Complex

let vect_schedule vect_optimized =
  let _ = info "vectorized scheduling..." in
  let vect_scheduled = schedule vect_optimized in
  let _ = info "vectorized annotating..." in
  let vect_annotated = annotate vect_scheduled in
  let _ = info "vectorized linearizing..." in
    annotatedscheduleToVsimdinstrs vect_annotated

let vect_optimize varinfo n dag =
  let _ = info "simplifying..." in
  let optim = Algsimp.algsimp dag in
  let alist = To_alist.to_assignments optim in

(*
  let _ = 
    List.iter
      (fun x -> Printf.printf "%s\n" (Expr.assignment_to_string x)) 
      alist
  in 
*)

  let _ = info "mapping assignments to vfpinstrs..." in
  let code1 = AssignmentsToVfpinstrs.assignmentsToVfpinstrs varinfo alist in
  let _ = info "vectorizing..." in
  let (operandsize,code2) = K7Vectorization.vectorize varinfo n code1 in

  let _ = info "optimizing..." in
  let code3 = VK7Optimization.apply_optrules code2 in

  let code4 = vect_schedule code3 in

  let _ = info "improving..." in
  let code5 = VImproveSchedule.improve_schedule code4 in

(*
  let _ = 
    List.iter
      (fun x -> Printf.printf "%s\n" (VFpUnparsing.vfpinstrToString x)) 
      code1
  in
*)

(*
  let _ = 
    List.iter
      (fun x -> Printf.printf "%s\n" (VSimdUnparsing.vsimdinstrToString x)) 
      code2
  in
*)
  (operandsize, code5)


(****************************************************************************)

let get2ndhalfcode base one dst ld_n = 
  if ld_n < 0 then
    [K7V_IntCpyUnaryOp(K7_ICopy, base, dst)]
    (* issue a warning or sth similar *)
    (* failwith "get2ndhalfcode: ld_n < 0 not supported!" *)
  else if ld_n <= 3 then
    [K7V_IntLoadEA(K7V_RISID(base,one,1 lsl ld_n,0), dst)]
  else
    [K7V_IntCpyUnaryOp(K7_ICopy, one, dst);
     K7V_IntUnaryOp(K7_IShlImm ld_n, dst);
     K7V_IntBinOp(K7_IAdd, base, dst)]


let loadfnargs xs = map (fun (src,dst) -> (dst, [K7V_IntLoadMem(src,dst)])) xs

(****************************************************************************)
let asmline x = print_string (x ^ "\n")

(* Warning: this function produces side-effects. *)
let emit_instr (_,_,instr) = 
  k7rinstrAdaptStackPointerAdjustment instr;
  asmline (K7Unparsing.k7rinstrToString instr)

let emit_instr' (t,cplen,i) = 
  k7rinstrAdaptStackPointerAdjustment i;
  Printf.printf "\t/*  t=%d, cp=%d  */\t" t cplen;
  print_string (K7Unparsing.k7rinstrToString i);
  print_newline ()

let emit_instr0' i = 
  k7rinstrAdaptStackPointerAdjustment i;
  Printf.printf "\t/*  t=?, cp=?  */\t";
  print_string (K7Unparsing.k7rinstrToString i);
  print_newline ()

let emit_instrk7v' i = 
  Printf.printf "\t/*  t=?, cp=?  */\t";
  print_string (K7Unparsing.k7vinstrToString i);
  print_newline ()

let emit_instrv' i = 
  Printf.printf "\t/*  t=?, cp=?  */\t";
  print_string (VSimdUnparsing.vsimdinstrToString i);
  print_newline ()

(****************************************************************************)


(* determine stackframe size in bytes *)
let getStackFrameSize instrs =
  let processInstr ((operand_size_bytes,max_bytes) as s0) = function
    | K7R_SimdSpill(_,idx) -> 
	(operand_size_bytes, max max_bytes ((idx+1) * operand_size_bytes))
    | K7R_SimdPromiseCellSize operand_size ->
	(k7operandsizeToInteger operand_size, max_bytes)
    | _ -> s0 in

  snd (fold_left processInstr (k7operandsizeToInteger K7_QWord,min_int) instrs)
			
(****************************************************************************)

type simdconstant = 
  | SC_NumberPair of Number.number * Number.number
  | SC_SimdPos of vsimdpos

let eq_numberpair (n,m) (n',m') = 
  Number.equal n n' && Number.equal m m'

let eq_simdconstant a b = match (a,b) with
  | (SC_SimdPos a, SC_SimdPos b) ->
      a=b
  | (SC_NumberPair(n,m), SC_NumberPair(n',m')) -> 
      eq_numberpair (n,m) (n',m')
  | _ -> 
      false

let k7rinstrsToConstants = 
  let rec rinstrsToConsts bag = function
    | [] -> List.rev bag
    | x::xs ->
	(match x with
	  | K7R_SimdUnaryOp(K7_FPChs p,_)
	    when not (exists (eq_simdconstant (SC_SimdPos p)) bag) ->
	      rinstrsToConsts ((SC_SimdPos p)::bag) xs 
	  | K7R_SimdUnaryOp(K7_FPMulConst(n,m),_)
	    when not (exists (eq_simdconstant (SC_NumberPair (n,m))) bag) ->
	      rinstrsToConsts ((SC_NumberPair (n,m))::bag) xs 
	  | _ -> rinstrsToConsts bag xs)
  in rinstrsToConsts []

let myconst_to_decl = function
  | SC_SimdPos V_Lo -> 
      (vsimdposToChsconstnamestring V_Lo)   ^ ": .long 0x80000000, 0x00000000"
  | SC_SimdPos V_Hi -> 
      (vsimdposToChsconstnamestring V_Hi)   ^ ": .long 0x00000000, 0x80000000"
  | SC_SimdPos V_LoHi -> 
      (vsimdposToChsconstnamestring V_LoHi) ^ ": .long 0x80000000, 0x80000000"
  | SC_NumberPair(n,m) ->
      sprintf "%s%s: .float %s, %s"
	      (Number.to_konst n)
	      (Number.to_konst m)
	      (Number.to_string n)
	      (Number.to_string m)

(****************************************************************************)

let instr_wants_down_pass1 = function
  | K7R_SimdPromiseCellSize _ -> false
  | K7R_SimdLoadStoreBarrier -> true
  | K7R_IntLoadMem _ -> true
  | K7R_IntStoreMem _ -> false
  | K7R_IntLoadEA _ -> true
  | K7R_IntUnaryOp _ -> true
  | K7R_IntUnaryOpMem _ -> true
  | K7R_IntCpyUnaryOp _ -> true
  | K7R_IntBinOp _ -> true
  | K7R_IntBinOpMem _ -> true
  | K7R_SimdLoad _ -> true
  | K7R_SimdStore _ -> false
  | K7R_SimdSpill _ -> false
  | K7R_SimdUnaryOp(K7_FPMulConst _,_) -> false
  | K7R_SimdUnaryOp _ -> true
  | K7R_SimdCpyUnaryOp _ -> true
  | K7R_SimdCpyUnaryOpMem _ -> true
  | K7R_SimdBinOp _ -> false
  | K7R_SimdBinOpMem _ -> false
  | K7R_Label _ -> false
  | K7R_Jump _ -> false
  | K7R_CondBranch _ -> false
  | K7R_FEMMS -> false
  | K7R_Ret -> false

let instr_wants_down_pass2 = function
  | K7R_SimdCpyUnaryOpMem _ -> false
  | instr -> instr_wants_down_pass1 instr

let rec move_down_instrs instr_wants_down = 
  let rec moveonedown i = function
    | [] -> [i]
    | x::xs as xxs -> 
	if k7rinstrCannotRollOverK7rinstr (get3of3 i) (get3of3 x) then 
	  i::xxs 
	else 
	  x::(moveonedown i xs)
  in function
    | [] -> []
    | x::xs -> 
	let xs' = move_down_instrs instr_wants_down xs in
	  if instr_wants_down (get3of3 x) then moveonedown x xs' else x::xs'

(****************************************************************************)

let k7rinstrIsAGIMovable = function
  | K7R_SimdLoad _ -> true
  | K7R_SimdStore _ -> true
  | K7R_SimdCpyUnaryOpMem _ -> true
  | K7R_SimdBinOpMem _ -> true
  | K7R_SimdSpill _ -> true
  | _ -> false

let k7rinstrCanMoveOverAGI a b = match (a,b) with
  | (K7R_SimdLoad _, K7R_SimdStore _) -> true
  | (K7R_SimdLoad _, K7R_SimdCpyUnaryOpMem _) -> true
  | (K7R_SimdLoad _, K7R_SimdSpill _) -> true
  | (K7R_SimdLoad _, K7R_SimdBinOpMem _) -> true
  | (K7R_SimdLoad _, K7R_SimdUnaryOp _) -> true
  | (K7R_SimdCpyUnaryOpMem _, K7R_SimdSpill _) -> true
  | (K7R_SimdUnaryOp _, K7R_SimdSpill _) -> true
  | (K7R_SimdStore _, K7R_SimdBinOpMem _) -> true
  | (K7R_SimdStore _, K7R_SimdCpyUnaryOpMem _) -> true
  | (K7R_SimdStore _, K7R_SimdSpill _) -> true
  | (K7R_SimdStore _, K7R_SimdUnaryOp _) -> true
  | _ -> false

let avoid_address_generation_interlock instrs = 
  let stop (_,_,x) (_,_,y) = 
    k7rinstrCannotRollOverK7rinstr x y || 
    (not (k7rinstrCanMoveOverAGI x y)) in
  let rec loop = function
    | [] -> []
    | ((_,_,instr) as x)::xs ->
	if k7rinstrIsAGIMovable instr then
	  insertList stop x (loop xs)
	else
	  x::(loop xs)
  in loop instrs

(****************************************************************************)

let k7rinstrsToPromiseEarly xs =
  let flag = ref false in
  let rec k7rinstrsToPromiseEarly__int = function
    | [] -> []
    | (K7R_Label _ as x1)::(K7R_SimdPromiseCellSize _ as x2)::xs ->
	x1::x2::(k7rinstrsToPromiseEarly__int xs)
    | x1::(K7R_SimdPromiseCellSize _ as x2)::xs ->
	flag := true;
	x2::(k7rinstrsToPromiseEarly__int (x1::xs))
    | x::xs ->
	x::(k7rinstrsToPromiseEarly__int xs)
  in (!flag,k7rinstrsToPromiseEarly__int xs)


let rec optimize ref_t0 instrs0 =
  let _ = info (sprintf "optimizing: len %d, ref_t %d." 
			(length instrs0) ref_t0) in
  let instrs1 = k7rinstrsToRegisterReallocated
		  (move_down_instrs 
		 	instr_wants_down_pass2
                        (move_down_instrs instr_wants_down_pass1 instrs0)) in
  let instrs = k7rinstrsToInstructionscheduled instrs1 in
  let (ref_t,_,_) = list_last instrs in
    if ref_t < ref_t0 then
      optimize ref_t instrs
    else
      instrs0

(****************************************************************************)

let rec addIndexToListElems i0 = function
  | [] -> []
  | x::xs -> (i0,x)::(addIndexToListElems (succ i0) xs)

let procedure_proepilog stackframe_size_bytes nargs code = 
  let regs_to_save =
	addIndexToListElems 1
		(filter (fun r -> exists (k7rinstrUsesK7rintreg r) code) 
			k7rintregs_calleesaved) in
  let total_stackframe_size = 
    0 + (if true || (nargs mod 2 = 1) then 4 else 0)
      + (if stackframe_size_bytes < 0 then 0 else stackframe_size_bytes) 
      + (if regs_to_save = [] then 0 else 16) in
  [K7R_FEMMS] @
  (if total_stackframe_size > 0 then 
     [K7R_IntUnaryOp(K7_ISubImm total_stackframe_size, k7rintreg_stackpointer)]
   else
     []) @
  (map (fun (i,r) -> K7R_IntStoreMem(r,K7_MFunArg (-i))) regs_to_save) @
  code @
  (map (fun (i,r) -> K7R_IntLoadMem(K7_MFunArg (-i), r)) regs_to_save) @
  (if total_stackframe_size > 0 then 
     [K7R_IntUnaryOp(K7_IAddImm total_stackframe_size, k7rintreg_stackpointer)]
   else
     []) @
  [K7R_FEMMS; K7R_Ret]

(****************************************************************************)
let datasection all_consts =
  if all_consts <> [] then begin
    asmline (".section .rodata");
    asmline ("\t" ^ ".balign 64");
    List.iter (fun c -> asmline (myconst_to_decl c)) all_consts;
    asmline (".text");
  end

let compileToAsm name nargs (initcode, k7vinstrs) =
  let _ = info "compileToAsm..." in
  let realcode = K7RegisterAllocator.regalloc initcode k7vinstrs in
  let stackframe_size_bytes = getStackFrameSize realcode in
  let realcode = procedure_proepilog stackframe_size_bytes nargs realcode in
  let _ = info "scheduling instructions..." in
  let realcode = fixpoint k7rinstrsToPromiseEarly realcode in
  let realcode' =
      avoid_address_generation_interlock 
	  (optimize 1000000 (k7rinstrsToInstructionscheduled realcode)) in

  let _ = info "before unparseToAsm" in
  let all_consts = k7rinstrsToConstants (map get3of3 realcode') in
  begin
    (* preserve Stefan's copyright, which otherwise does not appear
       anywhere else *)
    print_string ("/* The following asm code is Copyright (c) 2000-2001 Stefan Kral */\n");
    datasection all_consts;
    k7rinstrInitStackPointerAdjustment 0;
    asmline ".text";
    asmline "\t.balign 64";
    asmline (name ^ ":");
    iter emit_instr realcode';
  end

(******************************************************************)

let standard_arg_parse_fail _ = failwith "too many arguments"


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

let parse user_speclist usage =
  Arg.parse 
    (user_speclist @ speclist @ Magic.speclist)
    standard_arg_parse_fail 
    usage

let check_size () =
  match !size with
  | Some i -> i
  | None -> failwith "must specify -n"

let register_fcn name =  "fftwf_codelet_" ^ name

(* output the command line *)
let cmdline () =
  fold_right (fun a b -> a ^ " " ^ b) (Array.to_list Sys.argv) ""

let boilerplate cvsid =
  Printf.printf "%s"
    ("/* Generated by: " ^ (cmdline ()) ^ "*/\n\n" ^
     "/*\n" ^
     " * Generator Id's : \n" ^
     " * " ^ Algsimp.cvsid ^ "\n" ^
     " * " ^ Fft.cvsid ^ "\n" ^
     " * " ^ cvsid ^ "\n" ^
     " */\n\n")



let store_array_c n f =
  List.flatten
    (List.map (fun i -> store_var (access_output i) (f i)) (iota n))

let load_array_c n =
  array n (fun i -> load_var (access_input i))

let load_constant_array_c n =
  array n (fun i -> load_var (access_twiddle i))
