
include(../codelets.cmake)

set(fftw_dft_simd_sse2_codelist ${FFTW_dft_simd_codelets} codlist.c genus.c)
if(FFTW_MAINTENANCE_MODE)
  include(${CMAKE_SOURCE_DIR}/cmake/codelet_gen/dummy_codelet.cmake)

  foreach(_simd_codelet IN LISTS fftw_dft_simd_sse2_codelist)
    fftw_gen_simd_dummy(NAME ${_simd_codelet}
      HEADER "simd-sse2.h"
      ORIGDIR ${FFTW_dft_simd_common_dir}
      DESTDIR ${CMAKE_CURRENT_SOURCE_DIR}
    )
  endforeach()
endif(FFTW_MAINTENANCE_MODE)

if(HAVE_SSE2)
  add_library(fftw_dft_simd_sse2_objects OBJECT ${fftw_dft_simd_sse2_codelist})
  target_include_directories(fftw_dft_simd_sse2_objects PRIVATE ${FFTW_dft_simd_includes})
  target_compile_options(fftw_dft_simd_sse2_objects PRIVATE ${FFTW_SSE2_FLAGS})
  add_dependencies(fftw_dft_simd_sse2_objects fftw_dft_simd_common_codelets)
endif()
