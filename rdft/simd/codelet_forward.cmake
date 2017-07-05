
macro(fftw_dft_codelet_referrer _vec_extension _vec_ext_header)

include(../codelets.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/codelet_gen/dummy_codelet.cmake)

set(fftw_rdft_simd_${_vec_extension}_sources ${fftw_rdft_simd_codelets} genus.c codlist.c)

if(FFTW_MAINTENANCE_MODE)
  include(${CMAKE_SOURCE_DIR}/cmake/codelet_gen/dummy_codelet.cmake)
  foreach(_simd_codelet IN LISTS fftw_rdft_simd_${_vec_extension}_sources)
    fftw_gen_simd_dummy(NAME ${_simd_codelet}
      HEADER "${_vec_ext_header}"
      ORIGDIR ${FFTW_rdft_simd_common_dir}
      DESTDIR ${CMAKE_CURRENT_SOURCE_DIR}
    )
  endforeach()
endif(FFTW_MAINTENANCE_MODE)

if(HAVE_${_vec_extension})
  add_library(fftw_rdft_simd_${_vec_extension}_objects OBJECT ${fftw_rdft_simd_${_vec_extension}_sources})
  target_include_directories(fftw_rdft_simd_${_vec_extension}_objects PRIVATE ${FFTW_rdft_simd_include_dirs})
  target_compile_options(fftw_rdft_simd_${_vec_extension}_objects PRIVATE ${FFTW_${_vec_extension}_FLAGS})
  target_compile_definitions(fftw_rdft_simd_${_vec_extension}_objects PRIVATE ${FFTW_${_vec_extension}_DEFINE})
  add_dependencies(fftw_rdft_simd_${_vec_extension}_objects fftw_rdft_simd_common_codelets)
endif()

endmacro()
