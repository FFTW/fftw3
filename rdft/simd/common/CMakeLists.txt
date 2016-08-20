include(../codelets.cmake)


if(FFTW_MAINTENANCE_MODE)
  include(${CMAKE_SOURCE_DIR}/cmake/codelet_gen/make_codelets.cmake)
  include(${CMAKE_SOURCE_DIR}/cmake/codelet_gen/make_codlist.cmake)

  fftw_gen_codlist(
    XRENAME XSIMD
    SOLVTAB "XSIMD(solvtab_rdft)"
    HEADER "#include SIMD_HEADER\n"
    CODELETS ${fftw_rdft_simd_codelets}
    OUTPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/codlist.c
  )

  set(FLAGS_HC2C
    "-simd" ${GENFFT_FLAGS_COMMON} "-pipeline-latency" 8 "-trivial-stores"
    "-variables" 32 "-no-generate-bytw"
  )

  genfft_make_codelets(
    NAME hc2cfdftv
    SOURCES ${HC2CFDFTV}
    GENERATOR gen_hc2cdft_c
    ARGS ${FLAGS_HC2C} "-dit" "-include" "\"hc2cfv.h\""
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_rdft_prelude}
  )
  genfft_make_codelets(
    NAME hc2cbdftv
    SOURCES ${HC2CBDFTV}
    GENERATOR gen_hc2cdft_c
    ARGS ${FLAGS_HC2C} "-dif" "-sign" 1 "-include" "\"hc2cbv.h\""
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_rdft_prelude}
  )

endif(FFTW_MAINTENANCE_MODE)

add_custom_target(fftw_rdft_simd_common_codelets
  DEPENDS ${fftw_rdft_simd_codelets} codlist.c genus.c
)
