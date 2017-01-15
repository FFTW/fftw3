
macro(fftw_dft_codelet_referrer _vec_extension _vec_ext_header)

include(../codelets.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/codelet_gen/dummy_codelet.cmake)

set(fftw_dft_simd_${_vec_extension}_codelist ${FFTW_dft_simd_codelets} codlist.c genus.c)
if(FFTW_MAINTENANCE_MODE)
  include(${CMAKE_SOURCE_DIR}/cmake/codelet_gen/dummy_codelet.cmake)

  foreach(_simd_codelet IN LISTS fftw_dft_simd_${_vec_extension}_codelist)
    fftw_gen_simd_dummy(NAME ${_simd_codelet}
      HEADER "${_vec_ext_header}"
      ORIGDIR ${FFTW_dft_simd_common_dir}
      DESTDIR ${CMAKE_CURRENT_SOURCE_DIR}
    )
  endforeach()
endif(FFTW_MAINTENANCE_MODE)

if(HAVE_${_vec_extension})
  add_library(fftw_dft_simd_${_vec_extension}_objects OBJECT ${fftw_dft_simd_${_vec_extension}_codelist})
  target_include_directories(fftw_dft_simd_${_vec_extension}_objects PRIVATE ${FFTW_dft_simd_includes})
  target_compile_definitions(fftw_dft_simd_${_vec_extension}_objects PRIVATE ${FFTW_${_vec_extension}_DEFINES})
  target_compile_options(fftw_dft_simd_${_vec_extension}_objects PRIVATE ${FFTW_${_vec_extension}_FLAGS})
  add_dependencies(fftw_dft_simd_${_vec_extension}_objects fftw_dft_simd_common_codelets)
endif()

endmacro()
