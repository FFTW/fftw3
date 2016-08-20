set(reodft_source conf.c reodft.h reodft010e-r2hc.c
reodft11e-radix2.c reodft11e-r2hc-odd.c redft00e-r2hc-pad.c
rodft00e-r2hc-pad.c reodft00e-splitradix.c)

add_library(fftw_reodft_objects OBJECT ${reodft_source})
target_include_directories(fftw_reodft_objects PRIVATE
  ${fftw_config_path}
  ${CMAKE_SOURCE_DIR}/kernel
  ${CMAKE_SOURCE_DIR}/rdft
)
add_library(fftw_reodft_interface INTERFACE)
set_property(TARGET fftw_reodft_interface
  PROPERTY INTERFACE_SOURCES
  $<TARGET_OBJECTS:fftw_reodft_objects>
)
