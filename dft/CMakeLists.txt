
add_subdirectory(scalar)
add_subdirectory(simd)

set(fftw_dft_base_sources bluestein.c buffered.c conf.c ct.c dftw-direct.c
dftw-directsq.c dftw-generic.c dftw-genericbuf.c direct.c generic.c
indirect.c indirect-transpose.c kdft-dif.c kdft-difsq.c kdft-dit.c
kdft.c nop.c plan.c problem.c rader.c rank-geq2.c solve.c vrank-geq1.c
zero.c codelet-dft.h ct.h dft.h)


add_library(fftw_dft_common_objects OBJECT ${fftw_dft_base_sources})
target_include_directories(fftw_dft_common_objects
  PRIVATE
   ${fftw_config_path}
   ${CMAKE_SOURCE_DIR}/kernel
)

add_library(fftw_dft_interface INTERFACE )
set_property(TARGET fftw_dft_interface APPEND
  PROPERTY INTERFACE_SOURCES
  $<TARGET_OBJECTS:fftw_dft_common_objects>
  $<TARGET_OBJECTS:fftw_dft_scalar>
  $<TARGET_OBJECTS:fftw_dft_scalar_codelets>
)

macro(fftw_dft_simd_add_objects _vec_ext)
  if(HAVE_${_vec_ext})
    set_property(TARGET fftw_dft_interface APPEND
      PROPERTY INTERFACE_SOURCES
      $<TARGET_OBJECTS:fftw_dft_simd_${_vec_ext}_objects>
    )
  endif()
endmacro()

set(fftw_dft_vector_extensions ALTIVEC AVX NEON SSE2 AVX2 AVX_128_FMA)

foreach(_vector_ext IN LISTS fftw_dft_vector_extensions)
  fftw_dft_simd_add_objects(${_vector_ext})
endforeach()
