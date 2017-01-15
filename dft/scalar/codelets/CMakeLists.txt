include(codelets.cmake)

if(FFTW_MAINTENANCE_MODE)
  include(${CMAKE_SOURCE_DIR}/cmake/codelet_gen/make_codlist.cmake)
  include(${CMAKE_SOURCE_DIR}/cmake/codelet_gen/make_codelets.cmake)

  fftw_gen_codlist(
    XRENAME X
    SOLVTAB "X(solvtab_dft_standard)"
    CODELETS ${fftw_dft_scalar_codelets}
    OUTPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/codlist.c
  )

  set(DFT_FLAGS_COMMON  ${GENFFT_FLAGS_COMMON} -pipeline-latency 4)
  set(FLAGS_N1 ${DFT_FLAGS_COMMON} -include "\"n.h\"")
  set(FLAGS_T1 ${DFT_FLAGS_COMMON} -include "\"t.h\"")
  set(FLAGS_T2 ${DFT_FLAGS_COMMON} -twiddle-log3 -precompute-twiddles -include "\"t.h\"")
  set(FLAGS_F1 ${DFT_FLAGS_COMMON} -dif -include "\"f.h\"")
  set(FLAGS_F2 ${DFT_FLAGS_COMMON} -dif -twiddle-log3  -precompute-twiddles -include "\"f.h\"")
  set(FLAGS_Q1 ${DFT_FLAGS_COMMON} -dif -reload-twiddle -include "\"q.h\"")
  set(FLAGS_Q2 ${DFT_FLAGS_COMMON} -dif -twiddle-log3  -precompute-twiddles -include "\"q.h\"")


  genfft_make_codelets(NAME n1
    SOURCES ${N1_code}
    GENERATOR gen_notw
    ARGS ${FLAGS_N1}
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_dft_prelude}
  )

  genfft_make_codelets(NAME t1
    SOURCES ${T1_code}
    GENERATOR gen_twiddle
    ARGS ${FLAGS_T1}
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_dft_prelude}
  )

  genfft_make_codelets(NAME t2
    SOURCES ${T2_code}
    GENERATOR gen_twiddle
    ARGS ${FLAGS_T2}
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_dft_prelude}
  )

  genfft_make_codelets(NAME f1
    SOURCES ${F1_code}
    GENERATOR gen_twiddle
    ARGS ${FLAGS_F1}
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_dft_prelude}
  )

  genfft_make_codelets(NAME f2
    SOURCES ${F2_code}
    GENERATOR gen_twiddle
    ARGS ${FLAGS_F2}
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_dft_prelude}
  )

  genfft_make_codelets(NAME q1
    SOURCES ${Q1_code}
    GENERATOR gen_twidsq
    ARGS ${FLAGS_Q1}
    SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINDIR ${CMAKE_CURRENT_BINARY_DIR}
    PRELUDE ${FFTW_dft_prelude}
  )

endif()

add_library(fftw_dft_scalar_codelets OBJECT ${fftw_dft_scalar_codelets} ${CMAKE_CURRENT_SOURCE_DIR}/codlist.c)
target_include_directories(fftw_dft_scalar_codelets PRIVATE
  ${fftw_config_path}
  ${CMAKE_SOURCE_DIR}/kernel
  ${CMAKE_SOURCE_DIR}/dft
  ${CMAKE_SOURCE_DIR}/dft/scalar
)
