target_sources(${fftw3_lib}
  PRIVATE
    f.h
    n.h
    q.h
    t.h
    n.c
    t.c)

add_subdirectory(codelets)