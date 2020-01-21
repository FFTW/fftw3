set(THREAD_SOURCES_COMMON
    f77funcs.h
    threads.h
    api.c
    conf.c
    ct.c
    dft-vrank-geq1.c
    f77api.c
    hc2hc.c
    rdft-vrank-geq1.c
    vrank-geq1-rdft2.c)

if (ENABLE_THREADS)
  if (WITH_COMBINED_THREADS)
    target_sources(${fftw3_lib}
      PRIVATE
        ${THREAD_SOURCES_COMMON}
        threads.c)
  else ()
    target_sources(${fftw3_lib}_threads
      PRIVATE
        ${THREAD_SOURCES_COMMON}
        threads.c)
  endif ()
endif ()

if (ENABLE_OPENMP)
    target_sources(${fftw3_lib}_omp
      PRIVATE
        ${THREAD_SOURCES_COMMON}
        openmp.c)
endif ()