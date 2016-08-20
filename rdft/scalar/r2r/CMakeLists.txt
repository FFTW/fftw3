include(codelets.cmake)

if(FFTW_MAINTENANCE_MODE)
  include(${CMAKE_SOURCE_DIR}/cmake/codelet_gen/make_codelets.cmake)
  include(${CMAKE_SOURCE_DIR}/cmake/codelet_gen/make_codlist.cmake)

  fftw_gen_codlist(
    XRENAME X
    SOLVTAB "X(solvtab_rdft_r2r)"
    CODELETS ${fftw_rdft_r2r_scalar_codelets}
    OUTPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/codlist.c"
  )

  set(FLAGS_E01 ${RDFT_FLAGS_COMMON})
  set(FLAGS_E10 ${RDFT_FLAGS_COMMON})

  genfft_make_codelets(
    NAME e01
    SOURCES ${E01}
    GENERATOR gen_r2r
    ARGS ${FLAGS_E01} "-redft01" "-include" "\"r2r.h\""
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_rdft_prelude}
  )
  genfft_make_codelets(
    NAME e10
    SOURCES ${E10}
    GENERATOR gen_r2r
    ARGS ${FLAGS_E10} "-redft10" "-include" "\"r2r.h\""
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_rdft_prelude}
  )
endif(FFTW_MAINTENANCE_MODE)

add_library(fftw_rdft_scalar_r2r_objects OBJECT
  ${fftw_rdft_r2r_scalar_codelets} codlist.c
)
target_include_directories(fftw_rdft_scalar_r2r_objects PRIVATE
    ${fftw_config_path}
    ${CMAKE_SOURCE_DIR}/kernel
    ${CMAKE_SOURCE_DIR}/rdft
    ${CMAKE_SOURCE_DIR}/rdft/scalar
)
