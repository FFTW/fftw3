target_sources(${fftw3_lib}
  PRIVATE
    n1b.h
    n1f.h
    n2b.h
    n2f.h
    n2s.h
    q1b.h
    q1f.h
    t1b.h
    t1bu.h
    t1f.h
    t1fu.h
    t2b.h
    t2f.h
    t3b.h
    t3f.h
    ts.h)

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