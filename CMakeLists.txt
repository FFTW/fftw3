cmake_minimum_required (VERSION 3.12)

if (NOT DEFINED CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
endif ()

project (FFTW3
    VERSION 3.3.8
    HOMEPAGE_URL http://www.fftw.org/)

cmake_policy (SET CMP0042 NEW)
cmake_policy (SET CMP0076 NEW)

option (BUILD_SHARED_LIBS "Build shared libraries" ON)
option (BUILD_TESTS "Build tests" ON)

option (ENABLE_OPENMP "Use OpenMP for multithreading" OFF)
option (ENABLE_THREADS "Use pthread for multithreading" OFF)
option (WITH_COMBINED_THREADS "Merge thread library" OFF)

option (ENABLE_FLOAT "single-precision" OFF)
option (ENABLE_LONG_DOUBLE "long-double precision" OFF)
option (ENABLE_QUAD_PRECISION "quadruple-precision" OFF)

option (ENABLE_SSE "Compile with SSE instruction set support" OFF)
option (ENABLE_SSE2 "Compile with SSE2 instruction set support" OFF)
option (ENABLE_AVX "Compile with AVX instruction set support" OFF)
option (ENABLE_AVX2 "Compile with AVX2 instruction set support" OFF)

option (DISABLE_FORTRAN "Disable Fortran wrapper routines" OFF)

set(ENABLED_PRECISIONS 0)
foreach(SELECTED_PRECISION ENABLE_FLOAT ENABLE_LONG_DOUBLE ENABLE_QUAD_PRECISION)
    if(${SELECTED_PRECISION})
        math(EXPR ENABLED_PRECISIONS "${ENABLED_PRECISIONS} + 1")
    endif()
endforeach(SELECTED_PRECISION)
if(ENABLED_PRECISIONS GREATER 1)
    message(FATAL_ERROR "Only one or no (defaults to double precision) precision can be selected")
endif()

if((ENABLE_LONG_DOUBLE OR ENABLE_QUAD_PRECISION) AND (ENABLE_SSE2 OR ENABLE_AVX OR ENABLE_AVX2))
    message(FATAL_ERROR "ENABLE_SSE2, ENABLE_AVX and ENABLE_AVX2 not supported when ENABLE_LONG_DOUBLE or ENABLE_QUAD_PRECISION was selected")
endif()

include(GNUInstallDirs)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)

include (CheckIncludeFile)
check_include_file (alloca.h         HAVE_ALLOCA_H)
check_include_file (altivec.h        HAVE_ALTIVEC_H)
check_include_file (c_asm.h          HAVE_C_ASM_H)
check_include_file (dlfcn.h          HAVE_DLFCN_H)
check_include_file (intrinsics.h     HAVE_INTRINSICS_H)
check_include_file (inttypes.h       HAVE_INTTYPES_H)
check_include_file (libintl.h        HAVE_LIBINTL_H)
check_include_file (limits.h         HAVE_LIMITS_H)
check_include_file (mach/mach_time.h HAVE_MACH_MACH_TIME_H)
check_include_file (malloc.h         HAVE_MALLOC_H)
check_include_file (memory.h         HAVE_MEMORY_H)
check_include_file (stddef.h         HAVE_STDDEF_H)
check_include_file (stdint.h         HAVE_STDINT_H)
check_include_file (stdlib.h         HAVE_STDLIB_H)
check_include_file (string.h         HAVE_STRING_H)
check_include_file (strings.h        HAVE_STRINGS_H)
check_include_file (sys/types.h      HAVE_SYS_TYPES_H)
check_include_file (sys/time.h       HAVE_SYS_TIME_H)
check_include_file (sys/stat.h       HAVE_SYS_STAT_H)
check_include_file (sys/sysctl.h     HAVE_SYS_SYSCTL_H)
check_include_file (time.h           HAVE_TIME_H)
check_include_file (uintptr.h        HAVE_UINTPTR_H)
check_include_file (unistd.h         HAVE_UNISTD_H)
if (HAVE_TIME_H AND HAVE_SYS_TIME_H)
  set (TIME_WITH_SYS_TIME TRUE)
endif ()

include (CheckPrototypeDefinition) 
check_prototype_definition (drand48 "double drand48 (void)" "0" stdlib.h HAVE_DECL_DRAND48)
check_prototype_definition (srand48 "void srand48(long int seedval)" "0" stdlib.h HAVE_DECL_SRAND48)
check_prototype_definition (cosl "long double cosl( long double arg )" "0" math.h HAVE_DECL_COSL)
check_prototype_definition (sinl "long double sinl( long double arg )" "0" math.h HAVE_DECL_SINL)
check_prototype_definition (memalign "void *memalign(size_t alignment, size_t size)" "0" malloc.h HAVE_DECL_MEMALIGN)
check_prototype_definition (posix_memalign "int posix_memalign(void **memptr, size_t alignment, size_t size)" "0" stdlib.h HAVE_DECL_POSIX_MEMALIGN)

include (CheckSymbolExists)
check_symbol_exists (clock_gettime time.h HAVE_CLOCK_GETTIME)
check_symbol_exists (gettimeofday sys/time.h HAVE_GETTIMEOFDAY)
check_symbol_exists (getpagesize unistd.h HAVE_GETPAGESIZE)
check_symbol_exists (drand48 stdlib.h HAVE_DRAND48)
check_symbol_exists (srand48 stdlib.h HAVE_SRAND48)
check_symbol_exists (memalign malloc.h HAVE_MEMALIGN)
check_symbol_exists (posix_memalign stdlib.h HAVE_POSIX_MEMALIGN)
check_symbol_exists (mach_absolute_time mach/mach_time.h HAVE_MACH_ABSOLUTE_TIME)
check_symbol_exists (alloca alloca.h HAVE_ALLOCA)
if (NOT HAVE_ALLOCA)
  unset (HAVE_ALLOCA CACHE)
  check_symbol_exists (alloca malloc.h HAVE_ALLOCA)
endif ()
check_symbol_exists (isnan math.h HAVE_ISNAN)
check_symbol_exists (snprintf stdio.h HAVE_SNPRINTF)
check_symbol_exists (strchr string.h HAVE_STRCHR)
check_symbol_exists (sysctl unistd.h HAVE_SYSCTL)

if (UNIX)
  set (CMAKE_REQUIRED_LIBRARIES m)
endif ()
check_symbol_exists (cosl math.h HAVE_COSL)
check_symbol_exists (sinl math.h HAVE_SINL)

include (CheckTypeSize)
check_type_size ("float" SIZEOF_FLOAT)
check_type_size ("double" SIZEOF_DOUBLE)
check_type_size ("int" SIZEOF_INT)
check_type_size ("long" SIZEOF_LONG)
check_type_size ("long long" SIZEOF_LONG_LONG)
check_type_size ("unsigned int" SIZEOF_UNSIGNED_INT)
check_type_size ("unsigned long" SIZEOF_UNSIGNED_LONG)
check_type_size ("unsigned long long" SIZEOF_UNSIGNED_LONG_LONG)
check_type_size ("size_t" SIZEOF_SIZE_T)
check_type_size ("ptrdiff_t" SIZEOF_PTRDIFF_T)
math (EXPR SIZEOF_INT_BITS "8 * ${SIZEOF_INT}")
set (C_FFTW_R2R_KIND "C_INT${SIZEOF_INT_BITS}_T")

find_library (LIBM_LIBRARY NAMES m)
if (LIBM_LIBRARY)
  set (HAVE_LIBM TRUE)
endif ()


if (ENABLE_THREADS)
  find_package (Threads REQUIRED)
endif ()
if (Threads_FOUND)
  if(CMAKE_USE_PTHREADS_INIT)
    set(USING_POSIX_THREADS TRUE)
  endif ()
  set (HAVE_THREADS TRUE)
endif ()

if (ENABLE_OPENMP)
  find_package (OpenMP REQUIRED)
  set (HAVE_OPENMP TRUE)
endif ()

include (CheckCCompilerFlag)

if (ENABLE_SSE)
  foreach (FLAG "-msse" "/arch:SSE")
    unset (HAVE_SSE CACHE)
    check_c_compiler_flag (${FLAG} HAVE_SSE)
    if (HAVE_SSE)
      set (SSE_FLAG ${FLAG})
      break()
    endif ()
  endforeach ()
endif ()

if (ENABLE_SSE2)
  foreach (FLAG "-msse2" "/arch:SSE2")
    unset (HAVE_SSE2 CACHE)
    check_c_compiler_flag (${FLAG} HAVE_SSE2)
    if (HAVE_SSE2)
      set (SSE2_FLAG ${FLAG})
      break()
    endif ()
  endforeach ()
endif ()

if (ENABLE_AVX)
  foreach (FLAG "-mavx" "/arch:AVX")
    unset (HAVE_AVX CACHE)
    check_c_compiler_flag (${FLAG} HAVE_AVX)
    if (HAVE_AVX)
      set (AVX_FLAG ${FLAG})
      break()
    endif ()
  endforeach ()
endif ()

if (ENABLE_AVX2)
  foreach (FLAG "-mavx2" "/arch:AVX2")
    unset (HAVE_AVX2 CACHE)
    check_c_compiler_flag (${FLAG} HAVE_AVX2)
    if (HAVE_AVX2)
      set (AVX2_FLAG ${FLAG})
      break()
    endif ()
  endforeach ()
endif ()

# AVX2 codelets require FMA support as well
if (ENABLE_AVX2)
  foreach (FLAG "-mfma" "/arch:FMA")
    unset (HAVE_FMA CACHE)
    check_c_compiler_flag (${FLAG} HAVE_FMA)
    if (HAVE_FMA)
      set (FMA_FLAG ${FLAG})
      break()
    endif ()
  endforeach ()
endif ()

if (HAVE_SSE2 OR HAVE_AVX)
  set (HAVE_SIMD TRUE)
endif ()

set (PREC_SUFFIX)
if (ENABLE_FLOAT)
  set (FFTW_SINGLE TRUE)
  set (BENCHFFT_SINGLE TRUE)
  set (PREC_SUFFIX f)
endif ()

if (ENABLE_LONG_DOUBLE)
  set (FFTW_LDOUBLE TRUE)
  set (BENCHFFT_LDOUBLE TRUE)
  set (PREC_SUFFIX l)
endif ()

if (ENABLE_QUAD_PRECISION)
  set (FFTW_QUAD TRUE)
  set (BENCHFFT_QUAD TRUE)
  set (PREC_SUFFIX q)
endif ()
set (fftw3_lib fftw3${PREC_SUFFIX})

configure_file (cmake.config.h.in config.h @ONLY)
include_directories (${CMAKE_CURRENT_BINARY_DIR})

set(PUBLIC_HEADER_FILES
    "${CMAKE_CURRENT_LIST_DIR}/api/fftw3.h"
    "${CMAKE_CURRENT_BINARY_DIR}/fftw3_export.h")

add_library (${fftw3_lib})
add_library (${PROJECT_NAME}::${fftw3_lib} ALIAS ${fftw3_lib})

target_include_directories (${fftw3_lib}
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/api>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>)

set_target_properties(${fftw3_lib} PROPERTIES
    PUBLIC_HEADER "${PUBLIC_HEADER_FILES}"
    DEFINE_SYMBOL "fftw3_EXPORTS")

generate_export_header(${fftw3_lib}
    BASE_NAME "fftw3")

set (fftw3_libs ${fftw3_lib})
if (MSVC AND NOT (CMAKE_C_COMPILER_ID STREQUAL "Intel"))
  target_compile_definitions (${fftw3_lib} PRIVATE /bigobj)
endif ()
if (HAVE_SSE)
  target_compile_options (${fftw3_lib} PRIVATE ${SSE_FLAG})
endif ()
if (HAVE_SSE2)
  target_compile_options (${fftw3_lib} PRIVATE ${SSE2_FLAG})
endif ()
if (HAVE_AVX)
  target_compile_options (${fftw3_lib} PRIVATE ${AVX_FLAG})
endif ()
if (HAVE_AVX2)
  target_compile_options (${fftw3_lib} PRIVATE ${AVX2_FLAG})
endif ()
if (HAVE_FMA)
  target_compile_options (${fftw3_lib} PRIVATE ${FMA_FLAG})
endif ()
if (HAVE_LIBM)
  target_link_libraries (${fftw3_lib} m)
endif ()

set (subtargets ${fftw3_lib})

if (Threads_FOUND)
  target_compile_definitions (${fftw3_lib}
      PUBLIC
        ENABLE_THREADS)
  if (WITH_COMBINED_THREADS)
    target_link_libraries (${fftw3_lib} Threads::Threads)
  else ()
    add_library (${fftw3_lib}_threads)
    add_library (${PROJECT_NAME}::${fftw3_lib}_threads ALIAS ${fftw3_lib}_threads)
    set_target_properties(${fftw3_lib}_threads PROPERTIES
        DEFINE_SYMBOL "fftw3_threads_EXPORTS")
    generate_export_header(${fftw3_lib}_threads
        BASE_NAME "fftw3_threads")
    target_compile_definitions (${fftw3_lib}
        PRIVATE
            ENABLE_EXPORT_ADDITIONAL_FUNCTIONS)
    target_compile_definitions (${fftw3_lib}_threads
        PRIVATE
            ENABLE_EXPORT_ADDITIONAL_FUNCTIONS
            BUILD_THREADS_EXTERNAL)
    target_link_libraries (${fftw3_lib}_threads
        PUBLIC
            ${PROJECT_NAME}::${fftw3_lib}
        PRIVATE
            Threads::Threads)
    list (APPEND subtargets ${fftw3_lib}_threads)
  endif ()
endif ()

if (OpenMP_FOUND)
  add_library (${fftw3_lib}_omp)
  add_library (${PROJECT_NAME}::${fftw3_lib}_omp ALIAS ${fftw3_lib}_omp)
  set_target_properties(${fftw3_lib}_omp PROPERTIES
      DEFINE_SYMBOL "fftw3_threads_EXPORTS")
  generate_export_header(${fftw3_lib}_omp
      BASE_NAME "fftw3_threads")
  target_compile_definitions (${fftw3_lib}
      PUBLIC
          ENABLE_THREADS
      PRIVATE
          ENABLE_EXPORT_ADDITIONAL_FUNCTIONS)
  target_compile_definitions (${fftw3_lib}_omp
      PRIVATE
          ENABLE_EXPORT_ADDITIONAL_FUNCTIONS
          BUILD_THREADS_EXTERNAL)
  target_link_libraries (${fftw3_lib}_omp
    PUBLIC
        ${PROJECT_NAME}::${fftw3_lib}
        OpenMP::OpenMP_C)
  list (APPEND subtargets ${fftw3_lib}_omp)
endif ()

add_subdirectory(api)
add_subdirectory(dft)
add_subdirectory(kernel)
add_subdirectory(rdft)
add_subdirectory(reodft)
add_subdirectory(simd-support)
add_subdirectory(threads)

foreach(subtarget ${subtargets})
    set_target_properties (${subtarget} PROPERTIES
        LANGUAGES C
        SOVERSION 3.5.7
        C_STANDARD 11
        C_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN 1)
    target_include_directories (${subtarget}
      PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
      INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/api>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:include>)
endforeach()

install (TARGETS ${subtargets}
    EXPORT ${PROJECT_NAME}Targets
    ARCHIVE COMPONENT FFTW3_LIBRARY
    LIBRARY COMPONENT FFTW3_LIBRARY
    RUNTIME COMPONENT FFTW3_LIBRARY # This is for Windows
    PUBLIC_HEADER COMPONENT FFTW3_DEVELOPMENT)

if (EXISTS ${CMAKE_SOURCE_DIR}/api/fftw3.f)
  install (FILES api/fftw3.f api/fftw3l.f03 api/fftw3q.f03
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT FFTW3_DEVELOPMENT)
endif ()
if (EXISTS ${CMAKE_SOURCE_DIR}/api/fftw3.f03.in)
  file (READ api/fftw3.f03.in FFTW3_F03_IN OFFSET 42)
  file (WRITE ${CMAKE_CURRENT_BINARY_DIR}/fftw3.f03 "! Generated automatically.  DO NOT EDIT!\n\n")
  file (APPEND ${CMAKE_CURRENT_BINARY_DIR}/fftw3.f03 "  integer, parameter :: C_FFTW_R2R_KIND = ${C_FFTW_R2R_KIND}\n\n")
  file (APPEND ${CMAKE_CURRENT_BINARY_DIR}/fftw3.f03 "${FFTW3_F03_IN}")
  install (FILES ${CMAKE_CURRENT_BINARY_DIR}/fftw3.f03
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT FFTW3_DEVELOPMENT)
endif ()

if (BUILD_TESTS)

  add_executable (bench)
  add_subdirectory(libbench2)
  add_subdirectory(tests)

  target_include_directories (bench
    PRIVATE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>)

  if (Threads_FOUND)
      if(WITH_COMBINED_THREADS)
        target_link_libraries (bench ${PROJECT_NAME}::${fftw3_lib})
      else()
        target_link_libraries (bench ${PROJECT_NAME}::${fftw3_lib}_threads)
      endif()
  elseif (OpenMP_FOUND)
    target_link_libraries (bench ${PROJECT_NAME}::${fftw3_lib}_omp)
  else ()
    target_link_libraries (bench ${PROJECT_NAME}::${fftw3_lib})
  endif ()


  enable_testing ()

  if (Threads_FOUND)

    macro (fftw_add_test problem)
      add_test (NAME ${problem} COMMAND bench -s ${problem})
    endmacro ()

    fftw_add_test (32x64)
    fftw_add_test (ib256)

  endif ()
endif ()

# pkgconfig file
set (prefix ${CMAKE_INSTALL_PREFIX})
set (exec_prefix ${CMAKE_INSTALL_PREFIX})
set (libdir ${CMAKE_INSTALL_FULL_LIBDIR})
set (includedir ${CMAKE_INSTALL_FULL_INCLUDEDIR})
set (VERSION ${PROJECT_VERSION})
configure_file (fftw.pc.in fftw${PREC_SUFFIX}.pc @ONLY)
install (FILES
          ${CMAKE_CURRENT_BINARY_DIR}/fftw${PREC_SUFFIX}.pc
         DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
         COMPONENT FFTW3_DEVELOPMENT)

# cmake file
set (ConfigPackageLocation "${CMAKE_INSTALL_LIBDIR}/cmake/fftw3${PREC_SUFFIX}")

configure_file (FFTW3Config.cmake.in ${PROJECT_NAME}${PREC_SUFFIX}Config.cmake @ONLY)
write_basic_package_version_file(${PROJECT_NAME}${PREC_SUFFIX}ConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install (FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}${PREC_SUFFIX}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}${PREC_SUFFIX}ConfigVersion.cmake
    DESTINATION ${ConfigPackageLocation}
    COMPONENT FFTW3_DEVELOPMENT)

install(EXPORT ${PROJECT_NAME}Targets FILE ${PROJECT_NAME}${PREC_SUFFIX}Targets.cmake
  NAMESPACE ${PROJECT_NAME}${PREC_SUFFIX}::
  DESTINATION ${ConfigPackageLocation}
  COMPONENT FFTW3_DEVELOPMENT)


# CPack settings
set(CPACK_PACKAGE_NAME "libfftw3")
set(CPACK_PACKAGE_DESCRIPTION "Library for computing Fast Fourier Transforms")
set(CPACK_PACKAGE_CONTACT "Martin Stolpe martin.stolpe@iaf.fraunhofer.de")
set(CPACK_PACKAGE_VENDOR "Fraunhofer-Institut für Angewandte Festkörperphysik IAF")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "")
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_DEBIAN_FFTW3_LIBRARY_PACKAGE_NAME "libfftw3")
set(CPACK_DEBIAN_FFTW3_DEVELOPMENT_PACKAGE_NAME "libfftw3-dev")

include(CPack)

cpack_add_component(FFTW3_LIBRARY
    DISPLAY_NAME "libfftw3"
    DESCRIPTION "Library for computing Fast Fourier Transforms"
)
cpack_add_component(FFTW3_DEVELOPMENT
    DISPLAY_NAME "libfftw3-dev"
    DESCRIPTION "Library for computing Fast Fourier Transforms - development"
    DEPENDS FFTW3_LIBRARY
)
