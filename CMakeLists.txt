cmake_minimum_required(VERSION 3.18)

project(
  fftw
  VERSION 3.3.11
  DESCRIPTION "Fast Fourier transform library"
  LANGUAGES C)

# configure.ac uses libtool CURRENT:REVISION:AGE = 10:11:7.
set(FFTW_LIB_VERSION 3.7.11)

if(NOT CMAKE_BUILD_TYPE
   AND NOT CMAKE_CONFIGURATION_TYPES
   AND CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(CMAKE_BUILD_TYPE
      Release
      CACHE STRING "Build type" FORCE)
endif()

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_TOOLS "Build wisdom tools" ON)
option(BUILD_TESTS "Build tests" OFF)

option(ENABLE_MPI "Build MPI libraries" OFF)
option(ENABLE_OPENMP "Use OpenMP for multithreading" OFF)
option(ENABLE_THREADS "Use native threads for multithreading" OFF)
option(WITH_COMBINED_THREADS "Merge thread library" OFF)

option(ENABLE_DOUBLE "Build double-precision libraries" ON)
option(ENABLE_FLOAT "Build single-precision libraries" OFF)
option(ENABLE_LONG_DOUBLE "Build long-double precision libraries" OFF)
option(ENABLE_QUAD_PRECISION "Build quadruple-precision libraries" OFF)

option(ENABLE_SSE "Compile with SSE instruction set support (single precision)"
       OFF)
option(ENABLE_SSE2 "Compile with SSE/SSE2 instruction set support" OFF)
option(ENABLE_AVX "Compile with AVX instruction set support" OFF)
option(ENABLE_AVX2 "Compile with AVX2 and FMA instruction set support" OFF)
option(ENABLE_AVX512 "Compile with AVX512 instruction set support" OFF)
option(ENABLE_AVX_128_FMA "Compile with AVX128/FMA4 instruction set support"
       OFF)
option(ENABLE_KCVI "Compile with Knights Corner vector instruction support" OFF)
option(ENABLE_ALTIVEC "Compile with AltiVec support (single precision)" OFF)
option(ENABLE_VSX "Compile with IBM VSX instruction set support" OFF)
option(ENABLE_NEON "Compile with ARM NEON instruction set support" OFF)
option(ENABLE_SVE "Compile with ARM SVE instruction set support" OFF)
option(ENABLE_LSX "Compile with LoongArch LSX instruction set support" OFF)
option(ENABLE_LASX "Compile with LoongArch LASX instruction set support" OFF)
option(ENABLE_GENERIC_SIMD128 "Compile with generic 128-bit SIMD support" OFF)
option(ENABLE_GENERIC_SIMD256 "Compile with generic 256-bit SIMD support" OFF)

option(DISABLE_FORTRAN "Disable Fortran wrapper routines" OFF)
option(WITH_WINDOWS_F77_MANGLING "Use common Windows Fortran mangling styles"
       OFF)

if(NOT
   (ENABLE_DOUBLE
    OR ENABLE_FLOAT
    OR ENABLE_LONG_DOUBLE
    OR ENABLE_QUAD_PRECISION))
  message(FATAL_ERROR "Enable at least one precision")
endif()
set(FFTW_SIMD_BACKENDS
    SSE2
    AVX
    AVX2
    AVX512
    AVX_128_FMA
    KCVI
    ALTIVEC
    VSX
    NEON
    SVE
    LSX
    LASX
    GENERIC_SIMD128
    GENERIC_SIMD256)
foreach(simd SSE ${FFTW_SIMD_BACKENDS})
  if(ENABLE_${simd} AND NOT (ENABLE_DOUBLE OR ENABLE_FLOAT))
    message(FATAL_ERROR "ENABLE_${simd} requires single or double precision")
  endif()
endforeach()
if((ENABLE_SSE OR ENABLE_ALTIVEC) AND NOT ENABLE_FLOAT)
  message(FATAL_ERROR "ENABLE_SSE and ENABLE_ALTIVEC require single precision")
endif()
if(ENABLE_MPI
   AND NOT
       (ENABLE_DOUBLE
        OR ENABLE_FLOAT
        OR ENABLE_LONG_DOUBLE))
  message(FATAL_ERROR "MPI does not support quad precision")
endif()
if(WITH_COMBINED_THREADS AND (NOT ENABLE_THREADS OR ENABLE_OPENMP))
  message(
    FATAL_ERROR
      "WITH_COMBINED_THREADS requires ENABLE_THREADS and excludes ENABLE_OPENMP"
  )
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(cmake/checks.cmake)

set(HAVE_THREADS ${ENABLE_THREADS})
set(HAVE_OPENMP ${ENABLE_OPENMP})
set(USING_POSIX_THREADS FALSE)
if(ENABLE_THREADS)
  set(THREADS_PREFER_PTHREAD_FLAG TRUE)
  find_package(Threads REQUIRED)
  set(USING_POSIX_THREADS ${CMAKE_USE_PTHREADS_INIT})
endif()
if(ENABLE_OPENMP)
  find_package(OpenMP REQUIRED COMPONENTS C)
endif()

if(ENABLE_MPI)
  find_package(MPI REQUIRED COMPONENTS C)
endif()

# CMake consumes generated sources, just like a non-maintainer Autotools build.
foreach(dir dft/scalar/codelets rdft/scalar/r2cf rdft/scalar/r2cb
            rdft/scalar/r2r)
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/codlist.c")
    message(
      FATAL_ERROR
        "Missing generated codelets in ${dir}. Use a release tarball or bootstrap the development sources as described in README."
    )
  endif()
endforeach()

set(SOURCEFILES "")
foreach(
  dir
  api
  dft
  dft/scalar
  dft/scalar/codelets
  kernel
  rdft
  rdft/scalar
  rdft/scalar/r2cb
  rdft/scalar/r2cf
  rdft/scalar/r2r
  reodft
  simd-support)
  file(GLOB sources "${dir}/*.c")
  list(APPEND SOURCEFILES ${sources})
endforeach()

if(ENABLE_MPI)
  file(GLOB FFTW_MPI_SOURCES mpi/*.c)
  list(FILTER FFTW_MPI_SOURCES EXCLUDE REGEX
       "/(mpi-bench|testsched|f03-wrap)\\.c$")
  list(APPEND FFTW_MPI_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/mpi/f03-wrap.c")
endif()

set(FFTW_THREAD_SOURCES
    threads/api.c
    threads/conf.c
    threads/ct.c
    threads/dft-vrank-geq1.c
    threads/f77api.c
    threads/hc2hc.c
    threads/rdft-vrank-geq1.c
    threads/vrank-geq1-rdft2.c)
list(TRANSFORM FFTW_THREAD_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

if(BUILD_TESTS)
  enable_testing()
endif()
if(BUILD_TESTS OR BUILD_TOOLS)
  file(GLOB fftw_libbench2_SOURCE libbench2/*.c)
  list(REMOVE_ITEM fftw_libbench2_SOURCE
       "${CMAKE_CURRENT_SOURCE_DIR}/libbench2/useropt.c")
endif()

if(BUILD_TOOLS)
  set(PACKAGE fftw)
  set(VERSION ${PROJECT_VERSION})
  configure_file(tools/fftw-wisdom-to-conf.in fftw-wisdom-to-conf @ONLY)
  install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/fftw-wisdom-to-conf" TYPE BIN)
  install(FILES tools/fftw-wisdom-to-conf.1
          DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
endif()

install(FILES api/fftw3.h TYPE INCLUDE)
if(ENABLE_MPI)
  install(FILES mpi/fftw3-mpi.h TYPE INCLUDE)
endif()
include(cmake/fortran.cmake)

# Each directory isolates config.h and source-file properties for one precision.
set(FFTW_COMPONENTS "")
foreach(precision DOUBLE FLOAT LONG_DOUBLE QUAD_PRECISION)
  if(ENABLE_${precision})
    add_subdirectory(cmake ${precision})
  endif()
endforeach()

set(FFTW_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/fftw3")
install(
  EXPORT fftw3-targets
  NAMESPACE FFTW::
  DESTINATION ${FFTW_INSTALL_CMAKEDIR})

configure_package_config_file(
  cmake/fftw3-config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/cmake/fftw3-config.cmake"
  INSTALL_DESTINATION ${FFTW_INSTALL_CMAKEDIR}
  NO_SET_AND_CHECK_MACRO)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/fftw3-config-version.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMinorVersion)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cmake/fftw3-config.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/fftw3-config-version.cmake"
        DESTINATION ${FFTW_INSTALL_CMAKEDIR})
