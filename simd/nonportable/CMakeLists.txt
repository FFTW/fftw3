
add_library(fftw_simd_nonportable_objects OBJECT sse.c sse2.c)
target_compile_options(fftw_simd_nonportable_objects PRIVATE ${FFTW_SIMD_FLAGS})
target_include_directories(
  fftw_simd_nonportable_objects
  PRIVATE
  ${fftw_config_path}
  ${CMAKE_SOURCE_DIR}/kernel
  ${CMAKE_SOURCE_DIR}/simd
)
