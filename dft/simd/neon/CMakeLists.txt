
include(../codelet_forward.cmake)

fftw_dft_codelet_referrer(NEON simd-neon.h)

# include(../codelets.cmake)
#
# set(fftw_dft_simd_neon_sources ${FFTW_dft_simd_codelets} codlist.c genus.c)
# if(FFTW_MAINTENANCE_MODE)
#   include(${CMAKE_SOURCE_DIR}/cmake/codelet_gen/dummy_codelet.cmake)
#   foreach(_simd_codelet IN LISTS fftw_dft_simd_neon_sources)
#     fftw_gen_simd_dummy(NAME ${_simd_codelet}
#       HEADER "simd-neon.h"
#       ORIGDIR ${FFTW_dft_simd_common_dir}
#       DESTDIR ${CMAKE_CURRENT_SOURCE_DIR}
#     )
#   endforeach()
# endif(FFTW_MAINTENANCE_MODE)
#
# if(HAVE_NEON)
#   add_library(fftw_dft_simd_neon_objects OBJECT ${fftw_dft_simd_neon_sources})
#   target_include_directories(fftw_dft_simd_neon_objects PRIVATE ${FFTW_dft_simd_includes})
#   add_dependencies(fftw_dft_simd_neon_objects fftw_dft_simd_common_codelets)
# endif()
