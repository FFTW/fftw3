
add_library(fftw_tests_bench_objects OBJECT bench.c fftw-bench.c fftw-bench.h)
target_include_directories(fftw_tests_bench_objects PRIVATE
    ${fftw_config_path}
    ${CMAKE_SOURCE_DIR}/api
    ${CMAKE_SOURCE_DIR}/libbench2
)

add_executable(bench hook.c $<TARGET_OBJECTS:fftw_tests_bench_objects>)
target_include_directories(bench PRIVATE
    ${fftw_config_path}
    ${CMAKE_SOURCE_DIR}/kernel
    ${CMAKE_SOURCE_DIR}/libbench2
    ${CMAKE_SOURCE_DIR}/dft
    ${CMAKE_SOURCE_DIR}/rdft
    ${CMAKE_SOURCE_DIR}/reodft
    ${CMAKE_SOURCE_DIR}/threads
    ${CMAKE_SOURCE_DIR}/api
    )

set_target_properties(bench PROPERTIES OUTPUT_NAME "bench${fftw_precision_suffix}")
set_target_properties(bench PROPERTIES DEBUG_OUTPUT_NAME "bench${fftw_precision_suffix}-debug")
target_link_libraries(bench PUBLIC fftw3 bench2 ${FFTW_M_LIB})
if(NOT FFTW_COMBINED_THREADS)
  target_link_libraries(bench PUBLIC fftw3_threads)
endif()

if(FFTW_ENABLE_TESTS)
function(fftw_invoke_prob _name _prob)
  add_test(NAME "${_name}"
    COMMAND $<TARGET_FILE:bench> -o patient --verbose=2 --verify-rounds 5 --verify "${_prob}"
  )
endfunction()

function(fftw_test_geom _name _geom)
  fftw_invoke_prob("${_name}_if_${_geom}" "if${_geom}")
  fftw_invoke_prob("${_name}_of_${_geom}" "of${_geom}")
  fftw_invoke_prob("${_name}_ib_${_geom}" "ib${_geom}")
  fftw_invoke_prob("${_name}_ob_${_geom}" "ob${_geom}")
  fftw_invoke_prob("${_name}_dd_if_${_geom}" "//if${_geom}")
  fftw_invoke_prob("${_name}_dd_of_${_geom}" "//of${_geom}")
  fftw_invoke_prob("${_name}_dd_ib_${_geom}" "//ib${_geom}")
  fftw_invoke_prob("${_name}_dd_ob_${_geom}" "//ob${_geom}")
endfunction()

function(fftw_test_size _name _size)
  fftw_test_geom("${_name}" "c${_size}")
  fftw_test_geom("${_name}" "r${_size}")
endfunction()

set(fftw_powers_of_two 2 4 8 16 32 64 128 256 512 1024 2048 4096)

foreach(_fftsize IN LISTS fftw_powers_of_two)
  math(EXPR _fftsize_uneven "${_fftsize} + 5")
  fftw_test_size("one_dimension" ${_fftsize})
  fftw_test_size("one_dimension" ${_fftsize_uneven})
endforeach()

if(FFTW_TEST_ZERODIM)
  set(_test_count 0)
  foreach(_i RANGE 0 16)
    foreach(_j RANGE 0 16)
      foreach(_vl RANGE 1 5)
        math(EXPR _ivl "${_i} * ${_vl}")
        math(EXPR _jvl "${_j} * ${_vl}")
        set(_geom "${_i}:${_vl}:${_jvl}x${_j}:${_ivl}:${_vl}x${_vl}")

        fftw_invoke_prob("zero_dim01v_${_test_count}" "o1v${_geom}:1:1")
        fftw_invoke_prob("zero_dimi1v_${_test_count}" "i1v${_geom}:1:1")
        fftw_invoke_prob("zero_dimok1v_${_test_count}" "ok1v${_geom}:1:1")
        fftw_invoke_prob("zero_dimik1v_${_test_count}" "ik1v${_geom}:1:1")
        math(EXPR _test_count "${_test_count} + 1")
      endforeach()
    endforeach()
  endforeach()
endif()

endif(FFTW_ENABLE_TESTS)
