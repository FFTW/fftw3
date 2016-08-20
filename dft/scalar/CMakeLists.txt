
add_subdirectory(codelets)


add_library(fftw_dft_scalar OBJECT n.c t.c f.h n.h q.h t.h)
target_include_directories(fftw_dft_scalar PRIVATE
  ${fftw_config_path}
  ${CMAKE_SOURCE_DIR}/kernel
  ${CMAKE_SOURCE_DIR}/dft
)
