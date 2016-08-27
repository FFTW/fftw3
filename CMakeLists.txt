
cmake_minimum_required (VERSION 3.3)
project (libfftw3)

include(CheckCCompilerFlag)

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(FFTW_MAJOR_VERSION, "3.3")
set(FFTW_MINOR_VERSION, "4")
set(PACKAGE "fftw")
set(PACKAGE_VERSION "3.3.4")
set(VERSION "3.3.4")

# Configuration options

# -- Floating point precision

set(FFTW_KNOWN_PRECISIONS single double long quad )
set(FFTW_PRECISION "double" CACHE STRING "Compile FFTW for specified floating point precision")
set_property(CACHE FFTW_PRECISION PROPERTY STRINGS ${FFTW_KNOWN_PRECISIONS})
if(NOT ${FFTW_PRECISION} IN_LIST FFTW_KNOWN_PRECISIONS)
    message(FATAL_ERROR "invalid option: ${FFTW_PRECISION}")
endif()


# -- Vector unit

option(FFTW_SIMD_USE_ALTIVEC "Compile with Altivec SIMD code" OFF)
option(FFTW_SIMD_USE_AVX "Compile with AVX SIMD code" OFF)
option(FFTW_SIMD_USE_NEON "Compile with Neon SIMD code" OFF)
option(FFTW_SIMD_USE_SSE2 "Compile with SSE2 SIMD code" OFF)
option(FFTW_SIMD_USE_SSE "Compile with SSE SIMD code" OFF)

# -- fused multiply-add (FMA)

option(FFTW_FMA "Compile FFTW with optimizations for machines with fused multiply-add" OFF)
if(FFTW_FMA)
    set(HAVE_FMA 1)
endif()

# -- testing using ctest

option(FFTW_ENABLE_TESTS "Turn on test generation for ctest" OFF)


# -- build multi-threaded libs

option(FFTW_BUILD_THREADS "Compile FFTW with threading support" OFF)
option(FFTW_COMBINED_THREADS "Combine single and multi-threaded libraries" OFF)
option(FFTW_USE_OPENMP "Compile with OpenMP support" NO)


if(FFTW_USE_OPENMP)
  find_package(OpenMP REQUIRED)
  set(HAVE_OPENMP 1)
endif()

if(FFTW_BUILD_THREADS)
    set(CMAKE_THREAD_PREFER_PTHREAD ON)
    find_package(Threads REQUIRED)

    if(CMAKE_USE_PTHREADS_INIT)
        set(USING_POSIX_THREADS 1)
        set(HAVE_PTHREAD 1)
    endif()
endif()





# -- set options for precision (double is default)
if(FFTW_PRECISION STREQUAL "single")
    set(FFTW_SINGLE 1)
    set(BENCHFFT_SINGLE 1)
    set(fftw_precision_suffix "f")
elseif(FFTW_PRECISION STREQUAL "long")
    set(FFTW_LDOUBLE 1)
    set(BENCHFFT_LDOUBLE 1)
    set(fftw_precision_suffix "l")
elseif(FFTW_PRECISION STREQUAL "quad")
    set(FFTW_QUAD 1)
    set(BENCHFFT_QUAD 1)
    set(fftw_precision_suffix "q")
elseif(FFTW_PRECISION STREQUAL "double")
    set(fftw_precision_suffix "")
endif()


# ++ match precision and vector unit ++

if(FFTW_SIMD_USE_SSE)
  if(FFTW_PRECISION STREQUAL "single")
    set(HAVE_SSE 1)
    set(HAVE_SSE2 1)
  else()
    message(FATAL_ERROR "SSE requires single precision")
  endif()
endif()

set(fftw_sd_prec_list "single" "double")
if(FFTW_SIMD_USE_SSE2)
  if(FFTW_PRECISION IN_LIST fftw_sd_prec_list)
    set(HAVE_SSE 1)
    set(HAVE_SSE2 1)
  else()
    message(FATAL_ERROR "SSE2 requires single or double precision")
  endif()
endif()

if(FFTW_SIMD_USE_AVX)
  if(FFTW_PRECISION STREQUAL "double")
    set(HAVE_AVX 1)
  else()
    message(FATAL_ERROR "AVX requires double precision")
  endif()
endif()

if(FFTW_SIMD_USE_ALTIVEC)
  if(FFTW_PRECISION STREQUAL "single")
    set(HAVE_ALTIVEC 1)
  else()
    message(FATAL_ERROR "Altivec requires single precision")
  endif()
endif()

if(FFTW_SIMD_USE_NEON)
  if(FFTW_PRECISION STREQUAL "single")
    set(HAVE_NEON 1)
  else()
    message(FATAL_ERROR "ARM NEON requires single precision")
  endif()
endif()

if(HAVE_SSE OR HAVE_SSE2 OR HAVE_ALTIVEC OR HAVE_AVX OR HAVE_NEON)
  set(FFTW_WITH_SIMD 1)
else()
  unset(FFTW_WITH_SIMD)
endif()


# test system configuration

include (cmake/PrepareConfig.cmake)


# check if compile options match system prerequisites

if(FFTW_PRECISION STREQUAL "long" AND NOT HAVE_LONG_DOUBLE)
    message(FATAL_ERROR "Compiler does not support long double types")
endif()


if(FFTW_MAINTENANCE_MODE)
  find_package(OCaml REQUIRED)
  find_package(ClangFormat)
  set(FFTW_copyright_header ${CMAKE_SOURCE_DIR}/COPYRIGHT)
  set(FFTW_codelet_gen_root ${CMAKE_SOURCE_DIR}/cmake/codelet_gen)
  set(FFTW_make_codelets ${FFTW_codelet_gen_root}/make_codelets.cmake)
  set(FFTW_fftgen_fuse_script ${FFTW_codelet_gen_root}/fuse_files.cmake)
  set(FFTW_fftgen_rungenfft_script ${FFTW_codelet_gen_root}/runfftgen.cmake)
  set(FFTW_dft_prelude ${CMAKE_SOURCE_DIR}/support/codelet_prelude.dft)
  set(FFTW_rdft_prelude ${CMAKE_SOURCE_DIR}/support/codelet_prelude.rdft)
  set(GENFFT_FLAGS_COMMON "-compact" "-variables" 4)
  add_subdirectory(genfft)
endif(FFTW_MAINTENANCE_MODE)

if(HAVE_SSE)
  CHECK_C_COMPILER_FLAG("-msse" FFTW_CC_HAS_MSSE)
  if(FFTW_CC_HAS_MSSE2)
    set(FFTW_SSE_FLAGS "-msse")
  endif()
endif()

if(HAVE_SSE2)
  CHECK_C_COMPILER_FLAG("-msse2" FFTW_CC_HAS_MSSE2)
  if(FFTW_CC_HAS_MSSE2)
    set(FFTW_SSE2_FLAGS "-msse2")
  endif()
endif()

if(HAVE_AVX)
  CHECK_C_COMPILER_FLAG("-mavx" FFTW_CC_HAS_MAVX)
  if(FFTW_CC_HAS_MAVX)
    set(FFTW_AVX_FLAGS "-mavx")
  else()
    message(FATAL_ERROR "Can not set AVX compiler flag")
  endif()
endif()


set(FFTW_SIMD_FLAGS ${FFTW_AVX_FLAGS} ${FFTW_SSE2_FLAGS} ${FFTW_SSE_FLAGS})

# prepare config.h

set(FFTW_CC "${CMAKE_C_COMPILER} ${CMAKE_C_FLAGS}")
set(fftw_config_path ${CMAKE_CURRENT_BINARY_DIR})

configure_file(config.h.cmake ${fftw_config_path}/config.h)

add_subdirectory(api)
add_subdirectory(kernel)

add_subdirectory(dft)
add_subdirectory(rdft)
add_subdirectory(reodft)

if(FFTW_WITH_SIMD)
    add_subdirectory(simd-support)
endif()

set(fftw_lib_objects
  $<TARGET_OBJECTS:fftw_api_objects>
  $<TARGET_OBJECTS:fftw_kernel_objects>
)

if(FFTW_USE_OPENMP OR FFTW_BUILD_THREADS)
  add_subdirectory(threads)
endif()


add_library(fftw3 STATIC ${fftw_lib_objects})
target_sources(fftw3 PUBLIC ${CMAKE_SOURCE_DIR}/api/fftw3.h)
target_link_libraries(fftw3 PRIVATE
    fftw_dft_interface
    fftw_rdft_interface
    fftw_reodft_interface
)

if(FFTW_WITH_SIMD)
  target_link_libraries(fftw3 PRIVATE fftw_simd_support_interface)
endif()

if(FFTW_COMBINED_THREADS)
  target_link_libraries(fftw3 PRIVATE fftw_threads_interface)
endif()


install(TARGETS fftw3
        ARCHIVE DESTINATION lib)
install(FILES ${CMAKE_SOURCE_DIR}/api/fftw3.h DESTINATION include)


set_target_properties(fftw3 PROPERTIES OUTPUT_NAME "fftw3${fftw_precision_suffix}")
set_target_properties(fftw3 PROPERTIES DEBUG_OUTPUT_NAME "fftw3${fftw_precision_suffix}-debug")

add_subdirectory(libbench2)

if(FFTW_ENABLE_TESTS)
  enable_testing()
endif()

add_subdirectory(tests)
add_subdirectory(tools)
