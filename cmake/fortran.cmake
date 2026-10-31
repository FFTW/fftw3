# Interface files are installed even when legacy wrappers are disabled.
set(FFTW_FORTRAN_HEADERS api/fftw3.f api/fftw3l.f03 api/fftw3q.f03)
foreach(input ${FFTW_FORTRAN_HEADERS} api/fftw3.f03.in)
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${input}")
    message(
      FATAL_ERROR
        "Missing generated Fortran interface ${input}. Use a release tarball or bootstrap the development sources as described in README."
    )
  endif()
endforeach()

cmake_push_check_state(RESET)
set(CMAKE_REQUIRED_LIBRARIES ${FFTW_MATH_LIBRARIES})
set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/api/fftw3.h")
check_type_size("fftw_r2r_kind" SIZEOF_FFTW_R2R_KIND LANGUAGE C)
cmake_pop_check_state()
if(NOT SIZEOF_FFTW_R2R_KIND)
  message(FATAL_ERROR "Cannot determine sizeof(fftw_r2r_kind)")
endif()
math(EXPR FFTW_R2R_KIND_BITS "8 * ${SIZEOF_FFTW_R2R_KIND}")
set(C_FFTW_R2R_KIND "C_INT${FFTW_R2R_KIND_BITS}_T")

install(FILES ${FFTW_FORTRAN_HEADERS} TYPE INCLUDE)
file(READ api/fftw3.f03.in FFTW3_F03_IN)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/fftw3.f03"
     "  integer, parameter :: C_FFTW_R2R_KIND = ${C_FFTW_R2R_KIND}\n\n"
     "${FFTW3_F03_IN}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/fftw3.f03" TYPE INCLUDE)

if(ENABLE_MPI)
  cmake_push_check_state(RESET)
  set(CMAKE_REQUIRED_LIBRARIES MPI::MPI_C)
  set(CMAKE_EXTRA_INCLUDE_FILES mpi.h)
  check_type_size(MPI_Fint SIZEOF_MPI_FINT LANGUAGE C)
  cmake_pop_check_state()
  if(NOT SIZEOF_MPI_FINT)
    message(FATAL_ERROR "Cannot determine sizeof(MPI_Fint)")
  endif()
  math(EXPR MPI_FINT_BITS "8 * ${SIZEOF_MPI_FINT}")
  foreach(interface fftw3-mpi fftw3l-mpi)
    file(READ "mpi/${interface}.f03.in" contents)
    string(REPLACE C_MPI_FINT "C_INT${MPI_FINT_BITS}_T" contents "${contents}")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${interface}.f03" "${contents}")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${interface}.f03" TYPE INCLUDE)
  endforeach()
endif()

set(HAVE_F77_MANGLING FALSE)
set(F77_FUNC_EQUIV FALSE)
set(FFTW_G77_DEFAULT OFF)
if(NOT DISABLE_FORTRAN)
  # Before CMake 3.16, Fortran needs a patched Ninja executable.
  if(CMAKE_GENERATOR MATCHES "^Ninja" AND CMAKE_VERSION VERSION_LESS 3.16)
    message(STATUS "Fortran mangling detection with Ninja requires CMake 3.16+")
  else()
    include(CheckLanguage)
    check_language(Fortran)
    if(CMAKE_Fortran_COMPILER)
      enable_language(Fortran)
      include(FortranCInterface)
      set(HAVE_F77_MANGLING ${FortranCInterface_GLOBAL_FOUND})
    endif()
  endif()
endif()

if(HAVE_F77_MANGLING)
  set(F77_FUNC_EQUIV TRUE)
  foreach(part PREFIX SUFFIX CASE)
    if(NOT "${FortranCInterface_GLOBAL_${part}}" STREQUAL
       "${FortranCInterface_GLOBAL__${part}}")
      set(F77_FUNC_EQUIV FALSE)
    endif()
  endforeach()
  if(F77_FUNC_EQUIV
     AND (CMAKE_Fortran_COMPILER_ID STREQUAL "GNU"
          OR CMAKE_SYSTEM_NAME STREQUAL "GNU"
          OR CMAKE_LIBRARY_ARCHITECTURE MATCHES "gnu"))
    set(FFTW_G77_DEFAULT ON)
  endif()
elseif(NOT DISABLE_FORTRAN)
  message(STATUS "Using default Fortran wrappers")
endif()
option(WITH_G77_WRAPPERS "Add g77-compatible Fortran wrappers"
       ${FFTW_G77_DEFAULT})
set(WINDOWS_F77_MANGLING ${WITH_WINDOWS_F77_MANGLING})
