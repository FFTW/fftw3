
set(fftw_threads_common_sources
  api.c conf.c dft-vrank-geq1.c ct.c rdft-vrank-geq1.c hc2hc.c
  vrank-geq1-rdft2.c f77api.c f77funcs.h threads.h
)

set(fftw_threads_includes
  ${fftw_config_path}
  ${CMAKE_SOURCE_DIR}/kernel
  ${CMAKE_SOURCE_DIR}/dft
  ${CMAKE_SOURCE_DIR}/rdft
  ${CMAKE_SOURCE_DIR}/api
)

add_library(fftw_threads_interface INTERFACE)

if(FFTW_USE_OPENMP)

  add_library(fftw_threads_omp_objects OBJECT ${fftw_threads_common_sources} openmp.c)
  target_include_directories(fftw_threads_omp_objects PRIVATE ${fftw_threads_includes})
  target_compile_options(fftw_threads_omp_objects PUBLIC ${OpenMP_C_FLAGS})

  set_property(TARGET fftw_threads_interface PROPERTY INTERFACE_SOURCES
    $<TARGET_OBJECTS:fftw_threads_omp_objects>)

else()

  add_library(fftw_threads_objects OBJECT ${fftw_threads_common_sources} threads.c )
  target_include_directories(fftw_threads_objects PRIVATE ${fftw_threads_includes})

  set_property(TARGET fftw_threads_interface PROPERTY INTERFACE_SOURCES
    $<TARGET_OBJECTS:fftw_threads_objects>)

endif()

if(NOT FFTW_COMBINED_THREADS)
  if(FFTW_USE_OPENMP)
    add_library(fftw3_threads STATIC $<TARGET_OBJECTS:fftw_threads_omp_objects>)
  else()
    add_library(fftw3_threads STATIC $<TARGET_OBJECTS:fftw_threads_objects>)
  endif()

  set_target_properties(fftw3_threads PROPERTIES OUTPUT_NAME "fftw3${fftw_precision_suffix}_threads")
  set_target_properties(fftw3_threads PROPERTIES DEBUG_OUTPUT_NAME "fftw3${fftw_precision_suffix}_threads-debug")

endif()
