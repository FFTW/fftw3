include(codelets.cmake)

if(FFTW_MAINTENANCE_MODE)
  include(${CMAKE_SOURCE_DIR}/cmake/codelet_gen/make_codelets.cmake)
  include(${CMAKE_SOURCE_DIR}/cmake/codelet_gen/make_codlist.cmake)

  fftw_gen_codlist(
    XRENAME X
    SOLVTAB "X(solvtab_rdft_r2cb)"
    CODELETS ${fftw_rdft_scalar_codelets}
    OUTPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/codlist.c"
  )

  set(FLAGS_R2CB ${RDFT_FLAGS_COMMON} -sign 1)
  set(FLAGS_HB ${RDFT_FLAGS_COMMON} -sign 1)
  set(FLAGS_HB2 ${RDFT_FLAGS_COMMON} -sign 1 -twiddle-log3 -precompute-twiddles)
  set(FLAGS_HC2CB ${RDFT_FLAGS_COMMON} -sign 1)
  set(FLAGS_HC2CB2 ${RDFT_FLAGS_COMMON} -sign 1 -twiddle-log3 -precompute-twiddles)
  set(FLAGS_R2CBIII ${RDFT_FLAGS_COMMON} -sign 1)

  genfft_make_codelets(
    NAME r2cb
    SOURCES ${R2CB}
    GENERATOR gen_r2cb
    ARGS ${FLAGS_R2CB} "-include" "\"r2cb.h\""
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_rdft_prelude}
  )

  genfft_make_codelets(
    NAME hb
    SOURCES ${HB}
    GENERATOR gen_hc2hc
    ARGS ${FLAGS_HB} "-dif" "-include" "\"hb.h\""
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_rdft_prelude}
  )
  genfft_make_codelets(
    NAME hb2
    SOURCES ${HB2}
    GENERATOR gen_hc2hc
    ARGS ${FLAGS_HB2} "-dif" "-include" "\"hb.h\""
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_rdft_prelude}
  )
  genfft_make_codelets(
    NAME r2cbIII
    SOURCES ${R2CBIII}
    GENERATOR gen_r2cb
    ARGS ${FLAGS_R2CB} "-dft-III" "-include" "\"r2cbIII.h\""
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_rdft_prelude}
  )
  genfft_make_codelets(
    NAME hc2cb
    SOURCES ${HC2CB}
    GENERATOR gen_hc2c
    ARGS ${FLAGS_HC2CB} "-dif" "-include" "\"hc2cb.h\""
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_rdft_prelude}
  )
  genfft_make_codelets(
    NAME hc2cb2
    SOURCES ${HC2CB2}
    GENERATOR gen_hc2c
    ARGS ${FLAGS_HC2CB2} "-dif" "-include" "\"hc2cb.h\""
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_rdft_prelude}
  )
  genfft_make_codelets(
    NAME hc2cbdft
    SOURCES ${HC2CBDFT}
    GENERATOR gen_hc2cdft
    ARGS ${FLAGS_HC2CB} "-dif" "-include" "\"hc2cb.h\""
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_rdft_prelude}
  )
  genfft_make_codelets(
    NAME hc2cbdft2
    SOURCES ${HC2CBDFT2}
    GENERATOR gen_hc2cdft
    ARGS ${FLAGS_HC2CB} "-dif" "-include" "\"hc2cb.h\""
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_rdft_prelude}
  )
endif(FFTW_MAINTENANCE_MODE)

add_library(fftw_rdft_scalar_r2cb_objects OBJECT ${fftw_rdft_scalar_codelets} codlist.c)
target_include_directories(fftw_rdft_scalar_r2cb_objects PRIVATE
    ${fftw_config_path}
    ${CMAKE_SOURCE_DIR}/kernel
    ${CMAKE_SOURCE_DIR}/rdft
    ${CMAKE_SOURCE_DIR}/rdft/scalar
)
