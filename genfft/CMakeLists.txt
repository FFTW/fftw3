

set(genfft_mls
  algsimp.ml
  annotate.ml
  assoctable.ml
  c.ml
  complex.ml
  conv.ml
  dag.ml
  expr.ml
  fft.ml
  gen_hc2c.ml
  gen_hc2cdft.ml
  gen_hc2cdft_c.ml
  gen_hc2hc.ml
  gen_mdct.ml
  gen_notw.ml
  gen_notw_c.ml
  gen_r2cb.ml
  gen_r2cf.ml
  gen_r2r.ml
  gen_twiddle.ml
  gen_twiddle_c.ml
  gen_twidsq.ml
  gen_twidsq_c.ml
  genutil.ml
  littlesimp.ml
  magic.ml
  monads.ml
  number.ml
  oracle.ml
  schedule.ml
  simd.ml
  simdmagic.ml
  to_alist.ml
  trig.ml
  twiddle.ml
  unique.ml
  util.ml
  variable.ml
)

set(genfft_mlis
  algsimp.mli
  annotate.mli
  assoctable.mli
  c.mli
  complex.mli
  conv.mli
  dag.mli
  expr.mli
  fft.mli
  littlesimp.mli
  number.mli
  oracle.mli
  schedule.mli
  simd.mli
  to_alist.mli
  trig.mli
  twiddle.mli
  unique.mli
  util.mli
  variable.mli
)

set(ocaml_build_folder ${CMAKE_CURRENT_BINARY_DIR}/ocaml_build)

set(ocaml_native_compile_options
  "-lib" "Unix" "-lib" "Nums" "-j" "8"
  "-ocamlopt" "${OCaml_OCAMLOPTOPT_EXECUTABLE}"
  "-ocamldep" "${OCaml_OCAMLDEPOPT_EXECUTABLE}"
  "-ocamlc"   "${OCaml_OCAMLOPTOPT_EXECUTABLE}"
  "-cflag" "-O3" "-quiet")

file(MAKE_DIRECTORY ${ocaml_build_folder})

set(genfft_progs
  gen_hc2c
  gen_hc2cdft
  gen_hc2cdft_c
  gen_hc2hc
  gen_mdct
  gen_notw
  gen_notw_c
  gen_r2cb
  gen_r2cf
  gen_r2r
  gen_twiddle
  gen_twiddle_c
  gen_twidsq
  gen_twidsq_c
)

foreach(genfft_program IN LISTS genfft_progs)
  list(APPEND genfft_native_targets ${genfft_program}.native)
  list(APPEND genfft_native_builds ${ocaml_build_folder}/${genfft_program}.native)
endforeach()

add_custom_command(
  OUTPUT ${genfft_native_builds}
  COMMAND ${OCaml_OCAMLBUILDNATIVE_EXECUTABLE}
  "-build-dir" ${ocaml_build_folder}
  ${ocaml_native_compile_options}
  ${genfft_native_targets}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

foreach(genfft_program IN LISTS genfft_progs)
  add_custom_target(${genfft_program} ALL DEPENDS ${ocaml_build_folder}/${genfft_program}.native)
  set_property(TARGET ${genfft_program} PROPERTY OUTPUT_NAME ${ocaml_build_folder}/${genfft_program}.native)
endforeach()
