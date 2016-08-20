include(../codelets.cmake)

set(fftw_rdft_simd_sse2_sources ${fftw_rdft_simd_codelets} genus.c codlist.c)

if(FFTW_MAINTENANCE_MODE)
  include(${CMAKE_SOURCE_DIR}/cmake/codelet_gen/dummy_codelet.cmake)
  foreach(_simd_codelet IN LISTS fftw_rdft_simd_sse2_sources)
    fftw_gen_simd_dummy(NAME ${_simd_codelet}
      HEADER "simd-sse2.h"
      ORIGDIR ${FFTW_rdft_simd_common_dir}
      DESTDIR ${CMAKE_CURRENT_SOURCE_DIR}
    )
  endforeach()
endif(FFTW_MAINTENANCE_MODE)

if(HAVE_SSE2)
  add_library(fftw_rdft_simd_sse2_objects OBJECT ${fftw_rdft_simd_sse2_sources})
  target_include_directories(fftw_rdft_simd_sse2_objects PRIVATE ${FFTW_rdft_simd_include_dirs})
  target_compile_options(fftw_rdft_simd_sse2_objects PRIVATE ${FFTW_SSE2_FLAGS})
  add_dependencies(fftw_rdft_simd_sse2_objects fftw_rdft_simd_common_codelets)
endif()
