add_subdirectory(scalar)
add_subdirectory(simd)

set(RDFT2 buffered2.c direct2.c nop2.c rank0-rdft2.c rank-geq2-rdft2.c
plan2.c problem2.c solve2.c vrank-geq1-rdft2.c rdft2-rdft.c
rdft2-tensor-max-index.c rdft2-inplace-strides.c rdft2-strides.c
khc2c.c ct-hc2c.h ct-hc2c.c ct-hc2c-direct.c)

set(librdft_source hc2hc.h hc2hc.c dft-r2hc.c dht-r2hc.c dht-rader.c
buffered.c codelet-rdft.h conf.c direct-r2r.c direct-r2c.c generic.c
hc2hc-direct.c hc2hc-generic.c khc2hc.c kr2c.c kr2r.c indirect.c nop.c
plan.c problem.c rank0.c rank-geq2.c rdft.h rdft-dht.c solve.c
vrank-geq1.c vrank3-transpose.c)

list(APPEND librdft_source ${RDFT2})

add_library(fftw_rdft_common_objects OBJECT ${librdft_source})
target_include_directories(fftw_rdft_common_objects PRIVATE
    ${fftw_config_path}
    ${CMAKE_SOURCE_DIR}/kernel
    ${CMAKE_SOURCE_DIR}/dft)

add_library(fftw_rdft_interface INTERFACE)
set_property(TARGET fftw_rdft_interface APPEND
  PROPERTY INTERFACE_SOURCES
  $<TARGET_OBJECTS:fftw_rdft_common_objects>
  $<TARGET_OBJECTS:fftw_rdft_scalar_r2cb_objects>
  $<TARGET_OBJECTS:fftw_rdft_scalar_r2cf_objects>
  $<TARGET_OBJECTS:fftw_rdft_scalar_r2r_objects>
  $<TARGET_OBJECTS:fftw_rdft_scalar_common_objects>
)

if(HAVE_ALTIVEC)
  set_property(TARGET fftw_rdft_interface APPEND
    PROPERTY INTERFACE_SOURCES
    $<TARGET_OBJECTS:fftw_rdft_simd_altivec_objects>
  )
endif()

if(HAVE_AVX)
  set_property(TARGET fftw_rdft_interface APPEND
    PROPERTY INTERFACE_SOURCES
    $<TARGET_OBJECTS:fftw_rdft_simd_avx_objects>
  )
endif()

if(HAVE_NEON)
  set_property(TARGET fftw_rdft_interface APPEND
    PROPERTY INTERFACE_SOURCES
    $<TARGET_OBJECTS:fftw_rdft_simd_neon_objects>
  )
endif()

if(HAVE_SSE2)
  set_property(TARGET fftw_rdft_interface APPEND
    PROPERTY INTERFACE_SOURCES
    $<TARGET_OBJECTS:fftw_rdft_simd_sse2_objects>
  )
endif()
