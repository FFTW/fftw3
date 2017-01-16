
cmake_minimum_required (VERSION 3.3)
project (libfftw3)

include(CheckCCompilerFlag)

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(FFTW_MAJOR_VERSION, "3.3")
set(FFTW_MINOR_VERSION, "6")
set(PACKAGE "fftw")
set(PACKAGE_VERSION "3.3.6")
set(VERSION "3.3.6")

# Configuration options

# -- Maintainer settings

option(FFTW_MAINTENANCE_MODE "Enable Maintainer mode" OFF)

# -- Library flavor (shared / static)

option(FFTW_ENABLE_SHARED "Build shared library" OFF)
set(FFTW_ENABLE_STATIC ON)

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
option(FFTW_SIMD_USE_AVX2 "Compile with AVX2 SIMD code" OFF)
option(FFTW_SIMD_USE_NEON "Compile with Neon SIMD code" OFF)
option(FFTW_SIMD_USE_SSE2 "Compile with SSE2 SIMD code" OFF)
option(FFTW_SIMD_USE_SSE "Compile with SSE SIMD code" OFF)

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

option(FFTW_USE_FMA "Compile with FMA code" OFF)


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

if(FFTW_SIMD_USE_AVX2)
  if(FFTW_PRECISION STREQUAL "double")
    set(HAVE_AVX2 1)
  else()
    message(FATAL_ERROR "AVX2 requires double precision")
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


macro(fftw_check_flag _purpose _flag _var)
  unset(FFTW_CC_HAS_${_var})
  CHECK_C_COMPILER_FLAG(${_flag} FFTW_CC_HAS_${_var})
  if(FFTW_CC_HAS_${_var})
    set(${_var} ${_flag})
  else()
    message(FATAL_ERROR "Can not determine how to set ${_purpose}")
  endif()
endmacro()

if(HAVE_SSE)
  fftw_check_flag("SSE extension" "-msse" FFTW_SSE_FLAGS)
  if(FFTW_USE_FMA)
    set(FFTW_SSE_DEFINE "HAVE_FMA")
  endif()
endif()

if(HAVE_SSE2)
  fftw_check_flag("SSE2 extension" "-msse2" FFTW_SSE2_FLAGS)
  if(FFTW_USE_FMA)
    set(FFTW_SSE2_DEFINE "HAVE_FMA")
  endif()
endif()

if(HAVE_AVX)
  fftw_check_flag("AVX extension" "-mavx" FFTW_AVX_FLAGS)
  if(FFTW_USE_FMA)
    set(FFTW_AVX_DEFINE "HAVE_FMA")
  endif()
endif()

if(HAVE_AVX2)
  fftw_check_flag("AVX2 extension" "-mavx2" FFTW_AVX2_FLAGS)
  fftw_check_flag("FMA for AVX2" "-mfma" FFTW_FMA_FLAGS)
  set(FFTW_AVX2_DEFINES "HAVE_FMA")
  list(APPEND FFTW_AVX2_FLAGS ${FFTW_FMA_FLAGS})
endif()


set(FFTW_SIMD_FLAGS
    ${FFTW_AVX_FLAGS}
    ${FFTW_AVX2_FLAGS}
    ${FFTW_SSE2_FLAGS}
    ${FFTW_SSE_FLAGS}
  )
message(STATUS "CC flags are : ${FFTW_SIMD_FLAGS}")

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

if(FFTW_USE_OPENMP OR FFTW_BUILD_THREADS)
  add_subdirectory(threads)
endif()

set(fftw_lib_objects
  $<TARGET_OBJECTS:fftw_api_objects>
  $<TARGET_OBJECTS:fftw_kernel_objects>
)

set(fftw_lib_private_libs
  fftw_dft_interface
  fftw_rdft_interface
  fftw_reodft_interface
)

if(FFTW_WITH_SIMD)
  list(APPEND fftw_lib_private_libs fftw_simd_support_interface)
endif()

if(FFTW_COMBINED_THREADS)
  list(APPEND fftw_lib_private_libs fftw_threads_interface)
endif()

if(FFTW_ENABLE_STATIC)
  add_library(fftw3 STATIC ${fftw_lib_objects})
  target_sources(fftw3 PUBLIC ${CMAKE_SOURCE_DIR}/api/fftw3.h)
  target_link_libraries(fftw3 PRIVATE ${fftw_lib_private_libs})
  set_target_properties(fftw3 PROPERTIES OUTPUT_NAME "fftw3${fftw_precision_suffix}")
  set_target_properties(fftw3 PROPERTIES DEBUG_OUTPUT_NAME "fftw3${fftw_precision_suffix}-debug")
endif()

if(FFTW_ENABLE_SHARED)
  add_library(fftw3_shared SHARED ${fftw_lib_objects})
  target_sources(fftw3_shared PUBLIC ${CMAKE_SOURCE_DIR}/api/fftw3.h)
  target_link_libraries(fftw3_shared PRIVATE ${fftw_lib_private_libs})
  set_target_properties(fftw3_shared PROPERTIES OUTPUT_NAME "fftw3${fftw_precision_suffix}")
  set_target_properties(fftw3_shared PROPERTIES DEBUG_OUTPUT_NAME "fftw3${fftw_precision_suffix}-debug")
endif()

if(FFTW_ENABLE_SHARED)
  list(APPEND fftw_lib_targets fftw3_shared)
endif()
if(FFTW_ENABLE_STATIC)
  list(APPEND fftw_lib_targets fftw3)
endif()


add_subdirectory(libbench2)

if(FFTW_ENABLE_TESTS)
  enable_testing()
endif()

add_subdirectory(tests)
add_subdirectory(tools)


# -- Installation

install(TARGETS ${fftw_lib_targets}
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(FILES ${CMAKE_SOURCE_DIR}/api/fftw3.h DESTINATION include)



# -- Source code packaging

if(FFTW_MAINTENANCE_MODE)
add_custom_target(fftw_tarball
   COMMAND ${CMAKE_COMMAND}
   "-E" "tar" "cfz" "${CMAKE_BINARY_DIR}/fftw.tar.gz"
   "${CMAKE_SOURCE_DIR}/*"
   DEPENDS ${fftw_lib_targets}
   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
endif()
