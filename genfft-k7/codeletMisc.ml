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


open Util
open Fft

type codelet_type = 
  | TWIDDLE 
  | NO_TWIDDLE
  | REAL2HC
  | HC2HC
  | HC2REAL
  | REALEVEN
  | REALODD
  | REALEVEN2
  | REALODD2
  | REALEVEN_TWIDDLE
  | REALODD_TWIDDLE

(****************************************************************************)

let directionToInteger = function 	   (* values declared in ''fftw.h'' *)
  | Fft.FORWARD  -> -1
  | Fft.BACKWARD ->  1

let directionToHash = function      	   (* used for checksum calculation *)
  | Fft.FORWARD  -> 0
  | Fft.BACKWARD -> 1

let codelettypeToInteger = function        (* values declared in ''fftw.h'' *)
  | NO_TWIDDLE -> 0
  | TWIDDLE    -> 1
  | REAL2HC    -> 4
  | HC2HC      -> 6
  | HC2REAL    -> 5
  | _ 	       -> Util.info "codelettypeToInteger: type not supported!"; -1

let codelettypeToHash = function	   (* used for checksum calculation *)
  | TWIDDLE 	     -> 0
  | NO_TWIDDLE 	     -> 1
  | REAL2HC	     -> 2
  | HC2HC 	     -> 3
  | HC2REAL 	     -> 4
  | REALEVEN 	     -> 5
  | REALODD 	     -> 6
  | REALEVEN2 	     -> 7
  | REALODD2 	     -> 8
  | REALEVEN_TWIDDLE -> 9
  | REALODD_TWIDDLE  -> 10

let intsToAsmdeclstring = function
  | [] -> ""
  | xs -> "\t.long " ^ (listToString intToString "\n\t.long " xs) ^ "\n"

let codeletinfoToFnnameprefix = function
  | (NO_TWIDDLE, FORWARD)  -> "fftw_no_twiddle_"
  | (NO_TWIDDLE, BACKWARD) -> "fftwi_no_twiddle_"
  | (TWIDDLE, FORWARD)     -> "fftw_twiddle_"
  | (TWIDDLE, BACKWARD)    -> "fftwi_twiddle_"
  | (REAL2HC, _)	     -> "fftw_real2hc_"
  | (HC2REAL, _) 	     -> "fftw_hc2real_"
  | (HC2HC, FORWARD) 	     -> "fftw_hc2hc_forward_"
  | (HC2HC, BACKWARD)	     -> "fftw_hc2hc_backward_"
  | (REALEVEN, _)	     -> "fftw_realeven_"
  | (REALEVEN2, _)	     -> "fftw_realeven2_"
  | (REALODD, _)	     -> "fftw_realodd_"
  | (REALODD2, _)	     -> "fftw_realodd2_"
  | _ -> failwith "codeletinfoToFnnameprefix: unhandled codelet type!"

let codeletinfoToFnname n info = codeletinfoToFnnameprefix info ^ intToString n

let codelet_description n dir ctype =
  let name = codeletinfoToFnname n (ctype,dir) in
  let (_, num_twiddle, tw_o) = Twiddle.twiddle_policy () in
  let (declare_order, order, nt) = match ctype with
    | NO_TWIDDLE | REAL2HC | HC2REAL 
    | REALEVEN | REALODD | REALEVEN2 | REALODD2 -> 
	"", "0", 0
    | TWIDDLE | HC2HC | REALEVEN_TWIDDLE | REALODD_TWIDDLE -> 
	"\t.balign 4\n" ^
	"twiddle_order:\n" ^ (intsToAsmdeclstring (tw_o n)), 
	"twiddle_order",
	num_twiddle n

  (* this should be replaced by CRC/MD5 of the codelet *)
  and signature = 11 * (directionToHash dir + 2*n) + 
		  (codelettypeToHash ctype) in

  let n_str = intToString n
  and dir_str = intToString (directionToInteger dir)
  and ctype_str = intToString (codelettypeToInteger ctype)
  and sig_str = intToString signature
  and nt_str = intToString nt in
       ".section .rodata\n" ^ 
       declare_order ^
       "\n" ^
       "	.globl " ^ name ^ "_desc\n" ^ 
       "	.balign 32\n" ^
       name ^ "_desc:\n" ^ 
       "	/* descr_str_ptr */	.long .LC0\n" ^
       "	/* codelet_ptr */	.long " ^ name ^ "\n" ^
       "	/* n */			.long " ^ n_str ^ "\n" ^
       "	/* cdir_int */		.long " ^ dir_str ^ "\n" ^
       "	/* ctype_int */		.long " ^ ctype_str ^ "\n" ^ 
       "	/* signature */		.long " ^ sig_str ^ "\n" ^ 
       "	/* num_twiddles */	.long " ^ nt_str ^ "\n" ^
       "	/* twiddle_order */	.long " ^ order ^ "\n" ^
       ".LC0:	.string \"" ^ name ^ "\"\n"


