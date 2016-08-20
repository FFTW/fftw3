
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

if(HAVE_AVX)
  set_property(TARGET fftw_dft_interface APPEND
    PROPERTY INTERFACE_SOURCES
    $<TARGET_OBJECTS:fftw_dft_simd_avx_objects>
  )
endif()

if(HAVE_ALTIVEC)
  set_property(TARGET fftw_dft_interface APPEND
    PROPERTY INTERFACE_SOURCES
    $<TARGET_OBJECTS:fftw_dft_simd_altivec_objects>
  )
endif()

if(HAVE_NEON)
  set_property(TARGET fftw_dft_interface APPEND
    PROPERTY INTERFACE_SOURCES
    $<TARGET_OBJECTS:fftw_dft_simd_neon_objects>
  )
endif()

if(HAVE_SSE2)
  set_property(TARGET fftw_dft_interface APPEND
    PROPERTY INTERFACE_SOURCES
    $<TARGET_OBJECTS:fftw_dft_simd_sse2_objects>
  )
endif()
