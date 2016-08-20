
if(__DO_NOT_BUILD_ANYTHING_HERE_ITS_DEPRECATED__)

add_subdirectory(nonportable)

set(fftw_simdlib_source altivec.c sse.c sse2.c simd-altivec.h simd-sse.h
simd-sse2.h simd.h taint.c x86-cpuid.h mips_ps.c mips_ps.h simd-mips_ps.h)

add_library(fftw_simdlib_objects OBJECT ${fftw_simdlib_source})
target_include_directories(fftw_simdlib_objects PRIVATE
  ${fftw_config_path}
  ${CMAKE_SOURCE_DIR}/kernel
)
target_compile_options(fftw_simdlib_objects PRIVATE ${FFTW_SIMD_FLAGS})

add_library(fftw_simdlib_interface INTERFACE)
set_property(TARGET fftw_simdlib_interface
  PROPERTY INTERFACE_SOURCES
  $<TARGET_OBJECTS:fftw_simdlib_objects>
)

if(HAVE_SSE OR HAVE_SSE2)
  set_property(TARGET fftw_simdlib_interface APPEND
    PROPERTY INTERFACE_SOURCES
    $<TARGET_OBJECTS:fftw_simd_nonportable_objects>
  )
endif()


endif()
