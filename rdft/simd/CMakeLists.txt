
set(FFTW_rdft_simd_common_dir ${CMAKE_CURRENT_SOURCE_DIR}/common)
set(FFTW_rdft_simd_include_dirs
  ${fftw_config_path}
  ${CMAKE_SOURCE_DIR}/kernel
  ${CMAKE_SOURCE_DIR}/rdft
  ${CMAKE_SOURCE_DIR}/rdft/simd
  ${CMAKE_SOURCE_DIR}/simd-support
)

add_subdirectory(common)

add_subdirectory(altivec)
add_subdirectory(avx)
add_subdirectory(avx-128-fma)
add_subdirectory(avx2)
add_subdirectory(avx2-128)
add_subdirectory(avx512)
add_subdirectory(generic-simd128)
add_subdirectory(generic-simd256)
add_subdirectory(kcvi)
add_subdirectory(neon)
add_subdirectory(sse2)
add_subdirectory(vsx)
