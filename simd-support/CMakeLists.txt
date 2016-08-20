set(simd_support_source
    taint.c simd-common.h simd-sse2.h sse2.c
    x86-cpuid.h amd64-cpuid.h avx.c simd-avx.h altivec.c simd-altivec.h
    neon.c simd-neon.h )

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
