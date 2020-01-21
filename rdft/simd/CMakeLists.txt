target_sources(${fftw3_lib}
  PRIVATE
    hc2cbv.h
    hc2cfv.h)

if (HAVE_SSE2)
  add_subdirectory(sse2)
endif()
if (HAVE_AVX)
  add_subdirectory(avx)
endif ()
if (HAVE_AVX2)
  add_subdirectory(avx2)
  add_subdirectory(avx2-128)
endif ()