# Keep ISA flags local to codelets; runtime dispatch must remain portable.
set(SSE2_FLAGS "-msse2")
if(FFTW_SINGLE)
  set(SSE2_FLAGS "-msse")
  if(ENABLE_SSE)
    set(ENABLE_SSE2 ON)
  endif()
endif()
set(AVX_FLAGS "-mavx")
set(AVX2_FLAGS "-mavx2 -mfma")
set(AVX512_FLAGS "-mavx512f")
set(AVX_128_FMA_FLAGS "-mavx -mfma4")
set(KCVI_FLAGS "-mmic")
set(ALTIVEC_FLAGS "-maltivec")
set(VSX_FLAGS "-maltivec -mvsx")
set(NEON_FLAGS "-mfpu=neon")
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)$")
  set(NEON_FLAGS "")
endif()
set(SVE_FLAGS "-march=armv8-a+sve")
set(LSX_FLAGS "-mlsx")
set(LASX_FLAGS "-mlasx")
if(CMAKE_C_COMPILER_ID MATCHES "XL")
  set(ALTIVEC_FLAGS "-qaltivec")
  set(VSX_FLAGS "-qaltivec")
endif()
if(MSVC)
  set(SSE2_FLAGS "/arch:SSE2")
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(SSE2_FLAGS "")
  elseif(FFTW_SINGLE)
    set(SSE2_FLAGS "/arch:SSE")
  endif()
  set(AVX_FLAGS "/arch:AVX")
  set(AVX2_FLAGS "/arch:AVX2")
  set(AVX512_FLAGS "/arch:AVX512")
endif()

# Select all backends before generating the config.h used by the probes.
set(simd_backends "")
foreach(simd IN LISTS FFTW_SIMD_BACKENDS)
  set(HAVE_${simd} FALSE)
  if(NOT ENABLE_${simd}
     OR FFTW_LDOUBLE
     OR FFTW_QUAD
     OR (simd STREQUAL "ALTIVEC" AND NOT FFTW_SINGLE))
    continue()
  endif()
  if(simd STREQUAL "NEON"
     AND NOT FFTW_SINGLE
     AND NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)$")
    if(NOT ENABLE_FLOAT)
      message(FATAL_ERROR "NEON requires single precision on 32-bit ARM")
    endif()
    continue()
  endif()
  set(HAVE_${simd} TRUE)
  list(APPEND simd_backends ${simd})
endforeach()

cmake_push_check_state(RESET)
set(CMAKE_REQUIRED_LIBRARIES ${FFTW_MATH_LIBRARIES})
set(CMAKE_REQUIRED_INCLUDES "${CMAKE_CURRENT_BINARY_DIR}"
                            "${PROJECT_SOURCE_DIR}")
if(HAVE_ALTIVEC OR HAVE_VSX)
  set(CMAKE_REQUIRED_FLAGS "${ALTIVEC_FLAGS}")
  check_include_file(altivec.h FFTW_HAVE_ALTIVEC_H)
  set(HAVE_ALTIVEC_H ${FFTW_HAVE_ALTIVEC_H})
endif()
configure_file(cmake.config.h.in config.h @ONLY)
foreach(simd IN LISTS simd_backends)
  string(TOLOWER "${simd}" simd_dir)
  string(REPLACE "_" "-" simd_dir "${simd_dir}")
  set(simd_dirs ${simd_dir})
  if(simd STREQUAL "AVX2")
    list(APPEND simd_dirs avx2-128)
  elseif(simd STREQUAL "SVE")
    set(simd_dirs sve128 sve256 sve512 sve1024 sve2048)
    set_source_files_properties(
      "${PROJECT_SOURCE_DIR}/simd-support/sve-getwidth.c"
      PROPERTIES COMPILE_FLAGS "${SVE_FLAGS}")
  endif()
  set(CMAKE_REQUIRED_FLAGS "${${simd}_FLAGS}")
  foreach(dir IN LISTS simd_dirs)
    set(header "simd-${dir}.h")
    string(REPLACE "generic-simd" "generic" header "${header}")
    string(REPLACE "simd-sve" "simd-maskedsve" header "${header}")
    check_c_source_compiles(
      "#include \"kernel/ifftw.h\"
       #include \"simd-support/${header}\"
       int main(void) {
         R data[2 * VL] = {0};
         V x = LD(data, 2, data);
         x = VFMA(x, x, x);
         ST(data, x, 2, data);
         return (int)data[0];
       }"
      FFTW_${dir}${PREC_SUFFIX}_COMPILES)
    if(NOT FFTW_${dir}${PREC_SUFFIX}_COMPILES)
      message(
        FATAL_ERROR
          "ENABLE_${simd} requested, but the compiler cannot build ${header}")
    endif()
    foreach(kind dft rdft)
      if(NOT EXISTS "${PROJECT_SOURCE_DIR}/${kind}/simd/${dir}/codlist.c")
        message(FATAL_ERROR "Missing generated ${kind}/simd/${dir} codelets")
      endif()
      file(GLOB sources "${PROJECT_SOURCE_DIR}/${kind}/simd/${dir}/*.c")
      set_source_files_properties(${sources} PROPERTIES COMPILE_FLAGS
                                                        "${${simd}_FLAGS}")
      list(APPEND SOURCEFILES ${sources})
    endforeach()
  endforeach()
endforeach()
cmake_pop_check_state()
