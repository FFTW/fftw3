set(simd_support_source
  taint.c simd-common.h
  x86-cpuid.h amd64-cpuid.h
  simd-sse2.h sse2.c
  avx.c simd-avx.h
  avx-128-fma.c simd-avx-128-fma.h
  avx2.c simd-avx2.h simd-avx2-128.h
  avx512.c simd-avx512.h
  kcvi.c simd-kcvi.h
  altivec.c simd-altivec.h vsx.c simd-vsx.h
  neon.c simd-neon.h
  simd-generic128.h simd-generic256.h
)

add_library(fftw_simd_support_objects OBJECT ${simd_support_source})
target_compile_options(fftw_simd_support_objects PRIVATE ${FFTW_SIMD_FLAGS})
target_include_directories(fftw_simd_support_objects PRIVATE
    ${fftw_config_path}
    ${CMAKE_SOURCE_DIR}/kernel
)


add_library(fftw_simd_support_interface INTERFACE)
set_property(TARGET fftw_simd_support_interface
  PROPERTY INTERFACE_SOURCES
  $<TARGET_OBJECTS:fftw_simd_support_objects>
)

if(HAVE_SSE2)
  add_library(fftw_simd_support_sse2np_object OBJECT sse2-nonportable.c)
  target_compile_options(fftw_simd_support_sse2np_object PRIVATE ${FFTW_SSE2_FLAGS})
  target_include_directories(fftw_simd_support_sse2np_object PRIVATE
      ${fftw_config_path}
      ${CMAKE_SOURCE_DIR}/kernel)

  set_property(TARGET fftw_simd_support_interface APPEND
    PROPERTY INTERFACE_SOURCES
    $<TARGET_OBJECTS:fftw_simd_support_sse2np_object>
  )
endif()
