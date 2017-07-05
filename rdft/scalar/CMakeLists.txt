
if(FFTW_MAINTENANCE_MODE)
  set(RDFT_FLAGS_COMMON ${GENFFT_FLAGS_COMMON} -pipeline-latency 4)
endif(FFTW_MAINTENANCE_MODE)

add_subdirectory(r2cb)
add_subdirectory(r2cf)
add_subdirectory(r2r)

set(fftw_rdft_scalar_common_source
    hb.h r2cb.h r2cbIII.h hf.h hfb.c r2c.c
    r2cf.h r2cfII.h r2r.c r2r.h hc2c.c hc2cf.h hc2cb.h)


add_library(fftw_rdft_scalar_common_objects OBJECT ${fftw_rdft_scalar_common_source})
target_include_directories(fftw_rdft_scalar_common_objects PRIVATE
    ${fftw_config_path}
    ${CMAKE_SOURCE_DIR}/kernel
    ${CMAKE_SOURCE_DIR}/rdft)
