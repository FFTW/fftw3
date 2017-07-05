file(RELATIVE_PATH _codelet_fftgen_command ${CMAKE_SOURCE_DIR} ${FFT_GEN})

execute_process(COMMAND ${_codelet_fftgen_command} ${ARGS}
  OUTPUT_VARIABLE command_output
  RESULT_VARIABLE command_result
)

if(command_result)
  message(STATUS "return code: ${command_result}" )
  message(FATAL_ERROR "empty result from ${_codelet_fftgen_command} ${ARGS}")
else()
  file(WRITE ${RESULT_FILE} "${command_output}")
endif()
