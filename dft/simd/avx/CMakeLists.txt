include(../codelets.cmake)

set(fftw_dft_simd_avx_sources ${FFTW_dft_simd_codelets} codlist.c genus.c)
if(FFTW_MAINTENANCE_MODE)
  include(${CMAKE_SOURCE_DIR}/cmake/codelet_gen/dummy_codelet.cmake)
  foreach(_simd_codelet IN LISTS fftw_dft_simd_avx_sources)
    fftw_gen_simd_dummy(NAME ${_simd_codelet}
      HEADER "simd-avx.h"
      ORIGDIR ${FFTW_dft_simd_common_dir}
      DESTDIR ${CMAKE_CURRENT_SOURCE_DIR}
    )
  endforeach()
endif(FFTW_MAINTENANCE_MODE)

if(HAVE_AVX)
  add_library(fftw_dft_simd_avx_objects OBJECT ${fftw_dft_simd_avx_sources})
  target_include_directories(fftw_dft_simd_avx_objects PRIVATE ${FFTW_dft_simd_includes})
  target_compile_options(fftw_dft_simd_avx_objects PRIVATE ${FFTW_AVX_FLAGS})
  add_dependencies(fftw_dft_simd_avx_objects fftw_dft_simd_common_codelets)
endif()
