include(../codelets.cmake)


if(FFTW_MAINTENANCE_MODE)
  include(${CMAKE_SOURCE_DIR}/cmake/codelet_gen/make_codelets.cmake)
  include(${CMAKE_SOURCE_DIR}/cmake/codelet_gen/make_codlist.cmake)

  fftw_gen_codlist(
    XRENAME XSIMD
    SOLVTAB "XSIMD(solvtab_dft)"
    HEADER "#include SIMD_HEADER\n"
    CODELETS ${FFTW_dft_simd_codelets}
    OUTPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/codlist.c
  )

  set(GFLAGS "-simd" ${GENFFT_FLAGS_COMMON} "-pipeline-latency" "8")
  set(FLAGS_T2S "-twiddle-log3" "-precompute-twiddles")
  set(FLAGS_T3 "-twiddle-log3" "-precompute-twiddles" "-no-generate-bytw")

  genfft_make_codelets(
    NAME n1fv
    SOURCES ${FFTW_dft_simd_N1F}
    GENERATOR gen_notw_c
    ARGS ${GFLAGS} "-include" "\"n1f.h\""
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_dft_prelude}
  )
  genfft_make_codelets(
    NAME n2fv
    SOURCES ${FFTW_dft_simd_N2F}
    GENERATOR gen_notw_c
    ARGS ${GFLAGS} "-with-ostride" 2 "-include" "\"n2f.h\"" "-store-multiple" 2
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_dft_prelude}
  )
  genfft_make_codelets(
    NAME n1bv
    SOURCES ${FFTW_dft_simd_N1B}
    GENERATOR gen_notw_c
    ARGS ${GFLAGS} "-sign" 1 "-include" "\"n1b.h\""
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_dft_prelude}
  )
  genfft_make_codelets(
    NAME n2bv
    SOURCES ${FFTW_dft_simd_N2B}
    GENERATOR gen_notw_c
    ARGS ${GFLAGS} "-sign" 1 "-with-ostride" 2 "-include" "\"n2b.h\"" "-store-multiple" 2
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_dft_prelude}
  )
  genfft_make_codelets(
    NAME n2sv
    SOURCES ${FFTW_dft_simd_N2S}
    GENERATOR gen_notw
    ARGS ${GFLAGS} "-with-ostride" 1 "-include" "\"n2s.h\"" "-store-multiple" 4
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_dft_prelude}
  )
  genfft_make_codelets(
    NAME t1fv
    SOURCES ${FFTW_dft_simd_T1F}
    GENERATOR gen_twiddle_c
    ARGS ${GFLAGS} "-include" "\"t1f.h\""
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_dft_prelude}
  )
  genfft_make_codelets(
    NAME t1fuv
    SOURCES ${FFTW_dft_simd_T1FU}
    GENERATOR gen_twiddle_c
    ARGS ${GFLAGS} "-include" "\"t1fu.h\""
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_dft_prelude}
  )
  genfft_make_codelets(
    NAME t2fv
    SOURCES ${FFTW_dft_simd_T2F}
    GENERATOR gen_twiddle_c
    ARGS ${GFLAGS} "-include" "\"t2f.h\""
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_dft_prelude}
  )
  genfft_make_codelets(
    NAME t3fv
    SOURCES ${FFTW_dft_simd_T3F}
    GENERATOR gen_twiddle_c
    ARGS ${GFLAGS} ${FLAGS_T3} "-include" "\"t3f.h\""
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_dft_prelude}
  )
  genfft_make_codelets(
    NAME t1bv
    SOURCES ${FFTW_dft_simd_T1B}
    GENERATOR gen_twiddle_c
    ARGS ${GFLAGS} "-include" "\"t1b.h\"" "-sign" 1
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_dft_prelude}
  )
  genfft_make_codelets(
    NAME t1buv
    SOURCES ${FFTW_dft_simd_T1BU}
    GENERATOR gen_twiddle_c
    ARGS ${GFLAGS} "-include" "\"t1bu.h\"" "-sign" 1
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_dft_prelude}
  )
  genfft_make_codelets(
    NAME t2bv
    SOURCES ${FFTW_dft_simd_T2B}
    GENERATOR gen_twiddle_c
    ARGS ${GFLAGS} "-include" "\"t2b.h\"" "-sign" 1
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_dft_prelude}
  )
  genfft_make_codelets(
    NAME t3bv
    SOURCES ${FFTW_dft_simd_T3B}
    GENERATOR gen_twiddle_c
    ARGS ${GFLAGS} ${FLAGS_T3} "-include" "\"t3b.h\"" "-sign" 1
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_dft_prelude}
  )
  genfft_make_codelets(
    NAME t1sv
    SOURCES ${FFTW_dft_simd_T1S}
    GENERATOR gen_twiddle
    ARGS ${GFLAGS} "-include" "\"ts.h\""
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_dft_prelude}
  )
  genfft_make_codelets(
    NAME t2sv
    SOURCES ${FFTW_dft_simd_T2S}
    GENERATOR gen_twiddle
    ARGS ${GFLAGS} ${FLAGS_T2S} "-include" "\"ts.h\""
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_dft_prelude}
  )
  genfft_make_codelets(
    NAME q1fv
    SOURCES ${FFTW_dft_simd_Q1F}
    GENERATOR gen_twidsq_c
    ARGS ${GFLAGS} "-dif" "-include" "\"q1f.h\""
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_dft_prelude}
  )
  genfft_make_codelets(
    NAME q1bv
    SOURCES ${FFTW_dft_simd_Q1B}
    GENERATOR gen_twidsq_c
    ARGS ${GFLAGS} "-dif" "-include" "\"q1b.h\"" "-sign" 1
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_dft_prelude}
  )

endif()

add_custom_target(fftw_dft_simd_common_codelets
  DEPENDS ${FFTW_dft_simd_codelets} ${CMAKE_CURRENT_SOURCE_DIR}/codlist.c genus.c
)
