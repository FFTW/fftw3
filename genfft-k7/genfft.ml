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

(* This file contains the entry point for the genfft program: it
   parses the command-line parameters and calls the rest of the
   program as needed to generate the requested code. *)

open List
open Util
open Expr
open Variable
open Number


type mode = 
  | GEN_NOTHING
  | GEN_NOTWID of int
  | GEN_NOTWIDI of int
  | GEN_NOTWID_FIXED of int		(* with fixed stride *)
  | GEN_NOTWIDI_FIXED of int		(* with fixed stride *)
  | GEN_TWIDDLE of int
  | GEN_TWIDDLEI of int
  | GEN_REAL2HC of int
  | GEN_HC2REAL of int
  | GEN_HC2HC_FORWARD of int
  | GEN_HC2HC_BACKWARD of int
  | GEN_REAL_EVEN of int
  | GEN_REAL_ODD of int
  | GEN_REAL_EVEN2 of int
  | GEN_REAL_ODD2 of int
  | GEN_REAL_EVEN_TWIDDLE of int
  | GEN_REAL_ODD_TWIDDLE of int


let usage = "Usage: genfft [ -notwiddle | -notwiddleinv | -twiddle | -twiddleinv| -real2hc | -hc2hc-forward | -hc2hc-backward ] <number>"

let mode = ref GEN_NOTHING

let undocumented = " Undocumented voodoo parameter"

let fixed_istride = ref 1
let fixed_ostride = ref 1

let main () =
  Arg.parse [
  "-notwiddle", 
    Arg.Int(fun i -> mode := GEN_NOTWID i), 
    "<n> : Generate a no twiddle codelet of size <n>";
  "-notwiddleinv", 
    Arg.Int(fun i -> mode := GEN_NOTWIDI i), 
    "<n> : Generate a no twiddle inverse codelet of size <n>";

  "-notwiddle-with-fixedstride", 
    Arg.Int(fun i -> mode := GEN_NOTWID_FIXED i), 
    "<n> : Generate a no twiddle codelet of size <n> (w/fixed stride)";
  "-notwiddleinv-with-fixedstride", 
    Arg.Int(fun i -> mode := GEN_NOTWIDI_FIXED i), 
    "<n> : Generate a no twiddle inverse codelet of size <n> (w/fixed stride)";

  "-twiddle", 
    Arg.Int(fun i -> mode := GEN_TWIDDLE i), 
    "<n> : Generate a twiddle codelet of size <n>";
  "-twiddleinv", 
    Arg.Int(fun i -> mode := GEN_TWIDDLEI i), 
    "<n> : Generate a twiddle inverse codelet of size <n>";
  "-real2hc", 
    Arg.Int(fun i -> mode := GEN_REAL2HC i), 
    "<n> : Generate a real to halfcomplex codelet of size <n>";
  "-hc2hc-forward", 
    Arg.Int(fun i -> mode := GEN_HC2HC_FORWARD i), 
    "<n> : Generate a forward halfcomplex to halfcomplex codelet of size <n>";
  "-hc2hc-backward", 
    Arg.Int(fun i -> mode := GEN_HC2HC_BACKWARD i), 
    "<n> : Generate a backward halfcomplex to halfcomplex codelet of size <n>";
  "-hc2real", 
    Arg.Int(fun i -> mode := GEN_HC2REAL i), 
    "<n> : Generate a halfcomplex to real codelet of size <n>";
  "-realeven", 
    Arg.Int(fun i -> mode := GEN_REAL_EVEN i), 
    "<n> : Generate a real even codelet of size <n>";
  "-realodd", 
    Arg.Int(fun i -> mode := GEN_REAL_ODD i), 
    "<n> : Generate a real odd codelet of size <n>";
  "-realeven2", 
    Arg.Int(fun i -> mode := GEN_REAL_EVEN2 i), 
    "<n> : Generate a real even-2 codelet of size <n>";
  "-realodd2", 
    Arg.Int(fun i -> mode := GEN_REAL_ODD2 i), 
    "<n> : Generate a real odd-2 codelet of size <n>";
  "-realeventwiddle", 
    Arg.Int(fun i -> mode := GEN_REAL_EVEN_TWIDDLE i), 
    "<n> : Generate a real even twiddle codelet of size <n>";
  "-realoddtwiddle", 
    Arg.Int(fun i -> mode := GEN_REAL_ODD_TWIDDLE i), 
    "<n> : Generate a real odd twiddle codelet of size <n>";

  "-verbose", 
    Arg.Unit(fun () -> Magic.verbose := true),
    " Enable verbose logging messages to stderr";

  "-inline-konstants", 
    Arg.Unit(fun () -> Magic.inline_konstants := true),
    " Inline floating point constants";
  "-no-inline-konstants", 
    Arg.Unit(fun () -> Magic.inline_konstants := false),
    " Do not inline floating point constants";

  "-rader-min", Arg.Int(fun i -> Magic.rader_min := i),
    "<n> : Use Rader's algorithm for prime sizes >= <n>";

  "-with-istride", 
    Arg.Int(fun i -> fixed_istride := i),
    undocumented;
  "-with-ostride", 
    Arg.Int(fun i -> fixed_ostride := i),
    undocumented;

  "-magic-alternate-convolution", 
    Arg.Int(fun i -> Magic.alternate_convolution := i),
    undocumented;

  "-magic-window", 
    Arg.Int(fun i -> Magic.window := i), 
    undocumented;
  "-magic-variables", 
    Arg.Int(fun i -> Magic.number_of_variables := i),
    undocumented;

  "-magic-loopo", 
    Arg.Unit(fun () -> Magic.loopo := true),
    undocumented;
  "-magic-loopi", 
    Arg.Unit(fun () -> Magic.loopo := false),
    undocumented;

  "-magic-times-3-3", 
    Arg.Unit(fun () -> Magic.times_3_3 := true),
    undocumented;
  "-magic-times-4-2", 
    Arg.Unit(fun () -> Magic.times_3_3 := false),
    undocumented;

  "-magic-inline-single", 
    Arg.Unit(fun () -> Magic.inline_single := true),
    undocumented;
  "-magic-no-inline-single", 
    Arg.Unit(fun () -> Magic.inline_single := false),
    undocumented;

  "-magic-inline-loads", 
    Arg.Unit(fun () -> Magic.inline_loads := true),
    undocumented;
  "-magic-no-inline-loads", 
    Arg.Unit(fun () -> Magic.inline_loads := false),
    undocumented;

  "-magic-enable-fma", 
    Arg.Unit(fun () -> Magic.enable_fma := true),
    undocumented;
  "-magic-disable-fma", 
    Arg.Unit(fun () -> Magic.enable_fma := false),
    undocumented;

  "-magic-enable-fma-expansion", 
    Arg.Unit(fun () -> Magic.enable_fma_expansion := true),
    undocumented;
  "-magic-disable-fma-expansion", 
    Arg.Unit(fun () -> Magic.enable_fma_expansion := false),
    undocumented;

  "-magic-collect-common-twiddle", 
    Arg.Unit(fun () -> Magic.collect_common_twiddle := true),
    undocumented;
  "-magic-no-collect-common-twiddle", 
    Arg.Unit(fun () -> Magic.collect_common_twiddle := false),
    undocumented;

  "-magic-collect-common-inputs", 
    Arg.Unit(fun () -> Magic.collect_common_inputs := true),
    undocumented;
  "-magic-no-collect-common-inputs", 
    Arg.Unit(fun () -> Magic.collect_common_inputs := false),
    undocumented;

  "-magic-use-wsquare", 
    Arg.Unit(fun () -> Magic.use_wsquare := true),
    undocumented;
  "-magic-no-wsquare", 
    Arg.Unit(fun () -> Magic.use_wsquare := false),
    undocumented;

  "-magic-vectsteps-limit", 
    Arg.Int(fun i -> Magic.vectsteps_limit := i), 
    undocumented;

  "-magic-target-processor-k6",
    Arg.Unit(fun () -> Magic.target_processor := Magic.AMD_K6),
    " Produce code to run on an AMD K6-II+ (K6-III)";

  "-magic-twiddle-load-all", 
    Arg.Unit(fun () -> Magic.twiddle_policy := Magic.TWIDDLE_LOAD_ALL),
    undocumented;
  "-magic-twiddle-iter", 
    Arg.Unit(fun () -> Magic.twiddle_policy := Magic.TWIDDLE_ITER),
    undocumented;
  "-magic-twiddle-load-odd", 
    Arg.Unit(fun () -> Magic.twiddle_policy := Magic.TWIDDLE_LOAD_ODD),
    undocumented;
  "-magic-twiddle-square1", 
    Arg.Unit(fun () -> Magic.twiddle_policy := Magic.TWIDDLE_SQUARE1),
    undocumented;
  "-magic-twiddle-square2", 
    Arg.Unit(fun () -> Magic.twiddle_policy := Magic.TWIDDLE_SQUARE2),
    undocumented;
  "-magic-twiddle-square3", 
    Arg.Unit(fun () -> Magic.twiddle_policy := Magic.TWIDDLE_SQUARE3),
    undocumented;
] (fun _ -> failwith "too many arguments") usage;

  let code = 
    match !mode with
    | GEN_TWIDDLE i	   -> Gen_twiddle.twiddle_gen i Fft.FORWARD
    | GEN_TWIDDLEI i 	   -> Gen_twiddle.twiddle_gen i Fft.BACKWARD
    | GEN_NOTWID i 	   -> Gen_notwiddle.no_twiddle_gen i Fft.FORWARD
    | GEN_NOTWIDI i 	   -> Gen_notwiddle.no_twiddle_gen i Fft.BACKWARD
    | GEN_REAL2HC i 	   -> Gen_real2hc.real2hc_gen i
    | GEN_HC2HC_FORWARD i  -> Gen_hc2hc.hc2hc_gen i Fft.FORWARD
    | GEN_HC2REAL i 	   -> Gen_hc2real.hc2real_gen i
    | GEN_HC2HC_BACKWARD i -> Gen_hc2hc.hc2hc_gen i Fft.BACKWARD
    | GEN_REAL_EVEN i 	   -> Gen_realeven.realeven_gen i
    | GEN_REAL_EVEN2 i 	   -> Gen_realeven2.realeven2_gen i
    | GEN_REAL_ODD i 	   -> Gen_realodd.realodd_gen i
    | GEN_REAL_ODD2 i 	   -> Gen_realodd2.realodd2_gen i
    | GEN_NOTWID_FIXED i   -> 
      begin
	GenUtil.compileToAsm_with_fixedstride 
		(Gen_notwiddle_fixedstride.no_twiddle_gen_with_fixedstride 
			!fixed_istride 
			!fixed_ostride 
			i 
			Fft.FORWARD)
		(!fixed_istride, !fixed_ostride);
	exit 0
      end
    | GEN_NOTWIDI_FIXED i  ->
      begin
	GenUtil.compileToAsm_with_fixedstride
		(Gen_notwiddle_fixedstride.no_twiddle_gen_with_fixedstride
			!fixed_ostride 
			!fixed_ostride 
			i 
			Fft.BACKWARD)
		(!fixed_istride, !fixed_ostride);
	exit 0
      end

    | _ -> failwith "one of -notwiddle, -notwiddleinv, -twiddle, -twiddleinv, -real2hc, -hc2real, -hc2hc-forward, or -hc2hc-forward must be specified"

  in begin
    GenUtil.compileToAsm code;
    exit 0;
  end

let _ = main()

