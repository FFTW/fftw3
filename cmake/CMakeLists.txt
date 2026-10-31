# Instantiated once per enabled precision by the top-level CMakeLists.txt.
set(PREC_SUFFIX "")
foreach(kind SINGLE LDOUBLE QUAD)
  set(FFTW_${kind} FALSE)
endforeach()
if(precision STREQUAL "FLOAT")
  set(PREC_SUFFIX f)
  set(FFTW_SINGLE TRUE)
elseif(precision STREQUAL "LONG_DOUBLE")
  set(PREC_SUFFIX l)
  set(FFTW_LDOUBLE TRUE)
elseif(precision STREQUAL "QUAD_PRECISION")
  set(PREC_SUFFIX q)
  set(FFTW_QUAD TRUE)
endif()
foreach(kind SINGLE LDOUBLE QUAD)
  set(BENCHFFT_${kind} ${FFTW_${kind}})
endforeach()
set(fftw3_lib fftw3${PREC_SUFFIX})
if(HAVE_F77_MANGLING)
  FortranCInterface_HEADER(fortran-mangling.h MACRO_NAMESPACE "FFTW_FC_")
endif()
set(HAVE_LIBQUADMATH ${FFTW_QUAD})
if(FFTW_QUAD)
  list(APPEND FFTW_MATH_LIBRARIES quadmath)
endif()

set(HAVE_MPI FALSE)
if(ENABLE_MPI AND NOT FFTW_QUAD)
  set(HAVE_MPI TRUE)
endif()

include(simd.cmake)

add_library(${fftw3_lib} ${SOURCEFILES})
target_link_libraries(${fftw3_lib} PRIVATE ${FFTW_MATH_LIBRARIES})
target_include_directories(
  ${fftw3_lib} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/api>
                      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
# MinGW auto-exports the internal symbols used by the companion libraries.
if(BUILD_SHARED_LIBS AND NOT MINGW)
  target_compile_definitions(${fftw3_lib} PUBLIC FFTW_DLL)
endif()
set(subtargets ${fftw3_lib})
if(ENABLE_THREADS)
  set(fftw_threads_lib ${fftw3_lib})
  if(NOT WITH_COMBINED_THREADS)
    set(fftw_threads_lib ${fftw3_lib}_threads)
    add_library(${fftw_threads_lib})
    target_link_libraries(${fftw_threads_lib} PUBLIC ${fftw3_lib})
    list(APPEND subtargets ${fftw_threads_lib})
  endif()
  target_sources(
    ${fftw_threads_lib} PRIVATE ${FFTW_THREAD_SOURCES}
                                "${PROJECT_SOURCE_DIR}/threads/threads.c")
  target_link_libraries(${fftw_threads_lib} PRIVATE Threads::Threads)
endif()
if(ENABLE_OPENMP)
  add_library(${fftw3_lib}_omp ${FFTW_THREAD_SOURCES}
                               "${PROJECT_SOURCE_DIR}/threads/openmp.c")
  target_link_libraries(
    ${fftw3_lib}_omp
    PUBLIC ${fftw3_lib}
    PRIVATE $<BUILD_INTERFACE:OpenMP::OpenMP_C>
            $<INSTALL_INTERFACE:FFTW::OpenMP>)
  list(APPEND subtargets ${fftw3_lib}_omp)
endif()

if(HAVE_MPI)
  add_library(${fftw3_lib}_mpi ${FFTW_MPI_SOURCES})
  target_link_libraries(
    ${fftw3_lib}_mpi PUBLIC ${fftw3_lib} $<BUILD_INTERFACE:MPI::MPI_C>
                            $<INSTALL_INTERFACE:FFTW::MPI>)
  target_include_directories(
    ${fftw3_lib}_mpi PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/mpi>)
  list(APPEND subtargets ${fftw3_lib}_mpi)
endif()

foreach(subtarget IN LISTS subtargets)
  add_library(FFTW::${subtarget} ALIAS ${subtarget})
  target_include_directories(${subtarget} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
                                                  "${PROJECT_SOURCE_DIR}")
  if(MSVC)
    target_compile_options(${subtarget} PRIVATE /bigobj)
  endif()
endforeach()
set_target_properties(
  ${subtargets}
  PROPERTIES VERSION ${FFTW_LIB_VERSION}
             SOVERSION 3
             POSITION_INDEPENDENT_CODE ON)
if(APPLE)
  # Libtool uses CURRENT + 1, independently of the library's filename.
  set_target_properties(
    ${subtargets} PROPERTIES MACHO_COMPATIBILITY_VERSION 11
                             MACHO_CURRENT_VERSION 11.${PROJECT_VERSION_PATCH})
endif()
install(TARGETS ${subtargets} EXPORT fftw3-targets)

set(FFTW_COMPONENTS
    ${FFTW_COMPONENTS} ${subtargets}
    PARENT_SCOPE)

if(BUILD_TESTS OR BUILD_TOOLS)
  # A static archive lets wisdom override libbench2's command-line entry point.
  add_library(bench${PREC_SUFFIX}_lib STATIC ${fftw_libbench2_SOURCE})
  target_include_directories(
    bench${PREC_SUFFIX}_lib PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
                                    "${PROJECT_SOURCE_DIR}")
  function(fftw_add_program name library)
    list(TRANSFORM ARGN PREPEND "${PROJECT_SOURCE_DIR}/")
    add_executable(${name} "${PROJECT_SOURCE_DIR}/tests/fftw-bench.c" ${ARGN})
    target_include_directories(${name} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
                                               "${PROJECT_SOURCE_DIR}")
    target_link_libraries(${name} PRIVATE bench${PREC_SUFFIX}_lib ${library}
                                          ${FFTW_MATH_LIBRARIES})
    if(library STREQUAL "${fftw3_lib}_omp")
      target_link_libraries(${name} PRIVATE OpenMP::OpenMP_C)
    endif()
  endfunction()

  set(fftw_program_library ${fftw3_lib})
  if(ENABLE_THREADS)
    set(fftw_program_library ${fftw_threads_lib})
  elseif(ENABLE_OPENMP)
    set(fftw_program_library ${fftw3_lib}_omp)
  endif()
endif()

if(BUILD_TESTS)
  fftw_add_program(bench${PREC_SUFFIX} ${fftw_program_library} tests/bench.c
                   tests/hook.c)
  set(benchmarks bench${PREC_SUFFIX})
  if(ENABLE_THREADS AND ENABLE_OPENMP)
    fftw_add_program(bench${PREC_SUFFIX}_omp ${fftw3_lib}_omp tests/bench.c
                     tests/hook.c)
    list(APPEND benchmarks bench${PREC_SUFFIX}_omp)
  endif()
  foreach(bench IN LISTS benchmarks)
    foreach(problem 32x64 ib256)
      add_test(NAME ${bench}_${problem} COMMAND ${bench} -o estimate -y
                                                ${problem})
      if(ENABLE_THREADS OR ENABLE_OPENMP)
        add_test(NAME ${bench}_${problem}_threads
                 COMMAND ${bench} -o estimate -o nthreads=2 -y ${problem})
        set_tests_properties(${bench}_${problem}_threads PROPERTIES PROCESSORS
                                                                    2)
      endif()
    endforeach()
  endforeach()
  if(HAVE_MPI)
    set(bench mpi-bench${PREC_SUFFIX})
    fftw_add_program(${bench} ${fftw_program_library} mpi/mpi-bench.c
                     tests/hook.c)
    target_link_libraries(${bench} PRIVATE ${fftw3_lib}_mpi)
    function(fftw_add_mpi_test name nproc threads problem)
      if(nproc GREATER MPIEXEC_MAX_NUMPROCS)
        return()
      endif()
      add_test(
        NAME ${bench}_${name}
        COMMAND
          ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} ${nproc}
          ${MPIEXEC_PREFLAGS} $<TARGET_FILE:${bench}> ${MPIEXEC_POSTFLAGS} -o
          estimate -o nthreads=${threads} -y ${problem})
      math(EXPR processors "${nproc} * ${threads}")
      set_tests_properties(${bench}_${name} PROPERTIES PROCESSORS ${processors})
    endfunction()
    foreach(nproc 1 2 3 4)
      foreach(problem 32x16 ibr32x16)
        fftw_add_mpi_test(${nproc}_${problem} ${nproc} 1 ${problem})
      endforeach()
    endforeach()
    if(ENABLE_THREADS OR ENABLE_OPENMP)
      fftw_add_mpi_test(threads 2 2 32x16)
    endif()
  endif()
endif()

if(BUILD_TOOLS)
  fftw_add_program(fftw${PREC_SUFFIX}-wisdom ${fftw_program_library}
                   tests/bench.c tools/fftw-wisdom.c)
  install(TARGETS fftw${PREC_SUFFIX}-wisdom)
  configure_file("${PROJECT_SOURCE_DIR}/tools/fftw_wisdom.1.in"
                 fftw${PREC_SUFFIX}-wisdom.1 @ONLY)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/fftw${PREC_SUFFIX}-wisdom.1"
          DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
endif()

# Use CMake-specific templates; Autotools still consumes the top-level
# templates.
set(FFTW_PC_PRIVATE_LIBS "")
foreach(library IN LISTS FFTW_MATH_LIBRARIES)
  string(APPEND FFTW_PC_PRIVATE_LIBS " -l${library}")
endforeach()
if(WITH_COMBINED_THREADS)
  string(APPEND FFTW_PC_PRIVATE_LIBS " ${CMAKE_THREAD_LIBS_INIT}")
endif()
foreach(dir LIB INCLUDE)
  if(IS_ABSOLUTE "${CMAKE_INSTALL_${dir}DIR}")
    set(FFTW_PC_${dir}DIR "${CMAKE_INSTALL_${dir}DIR}")
  else()
    set(FFTW_PC_${dir}DIR "\${prefix}/${CMAKE_INSTALL_${dir}DIR}")
  endif()
endforeach()
configure_file(fftw.pc.in ${fftw3_lib}.pc @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${fftw3_lib}.pc"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
