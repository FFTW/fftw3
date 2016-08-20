
function(genfft_make_codelets)
  set(_keywords "NAME" "SOURCES" "GENERATOR" "ARGS" "SRCDIR" "BINDIR" "PRELUDE")
  set(_lastkeyword "")
  unset(_codelet_basename)
  unset(_codelet_sources)
  unset(_codelet_generator)
  unset(_codelet_generator_args)
  unset(_codelet_binary_dir)
  unset(_codelet_source_dir)
  unset(_codelet_prelude_file)
  unset(_codelet_format_cmd)
  foreach(arg IN LISTS ARGN)
    if(arg IN_LIST _keywords)
      set(_lastkeyword ${arg})
      continue()
    elseif(_lastkeyword STREQUAL "NAME")
       if(_codelet_basename)
         message(FATAL_ERROR "Name can only be given one argument")
       else()
         set(_codelet_basename ${arg})
       endif()
    elseif(_lastkeyword STREQUAL "SOURCES")
      list(APPEND _codelet_sources ${arg})
    elseif(_lastkeyword STREQUAL "GENERATOR")
      if(_codelet_generator)
        message(FATAL_ERROR "GENERATOR expects one argument")
      else()
        set(_codelet_generator ${arg})
      endif()
    elseif(_lastkeyword STREQUAL "ARGS")
      list(APPEND _codelet_generator_args ${arg})
    elseif(_lastkeyword STREQUAL "SRCDIR")
      if(_codelet_source_dir)
        message(FATAL_ERROR "SRCDIR expects a single argument")
      else()
        set(_codelet_source_dir ${arg})
      endif()
    elseif(_lastkeyword STREQUAL "BINDIR")
      if(_codelet_binary_dir)
        message(FATAL_ERROR "BINDIR expects a single argument")
      else()
        set(_codelet_binary_dir ${arg})
      endif()
    elseif(_lastkeyword STREQUAL "PRELUDE")
      if(_codelet_prelude_file)
        message(FATAL_ERROR "PRELUDE expects a single argument")
      else()
        set(_codelet_prelude_file ${arg})
      endif()
    endif()
  endforeach()

  if(NOT _codelet_basename)
    message(FATAL_ERROR "Codelet name is required!")
  endif()

  if(NOT _codelet_sources)
    message(FATAL_ERROR "Not codelet sources specified")
  endif()

  if(NOT _codelet_generator)
    message(FATAL_ERROR "Not codelet generator specified")
  endif()

  if(NOT _codelet_source_dir)
    message(FATAL_ERROR "Binary dir not set")
  endif()

  if(NOT _codelet_source_dir)
    message(FATAL_ERROR "Source directory not set")
  endif()

  if(NOT _codelet_prelude_file)
    message(FATAL_ERROR "No prelude file given")
  endif()

  foreach(_codelet_codelet_source IN LISTS _codelet_sources)
    unset(_codelet_codelet_name)
    get_filename_component(_codelet_codelet_name ${_codelet_codelet_source} NAME_WE)
    if (${_codelet_codelet_name} MATCHES "${_codelet_basename}_([0-9]+)")
      set( codelet_n ${CMAKE_MATCH_1})
    else()
      message(FATAL_ERROR "can not get codelet n from ${_codelet_codelet_name}")
    endif()
    set(_codelet_tmp_nofma_file ${_codelet_binary_dir}/${_codelet_codelet_name}.tmp)
    set(_codelet_tmp_fma_file ${_codelet_binary_dir}/${_codelet_codelet_name}_fma.tmp)
    set(_codelet_fft_generator $<TARGET_PROPERTY:${_codelet_generator},OUTPUT_NAME>)
    set(_codelet_nofma_args ${_codelet_generator_args} -n ${codelet_n} -name ${_codelet_codelet_name})
    set(_codelet_fma_args ${_codelet_nofma_args} -fma -reorder-insns -schedule-for-pipeline)

    add_custom_command(OUTPUT ${_codelet_tmp_nofma_file}
    COMMAND ${CMAKE_COMMAND}
    "-DFFT_GEN=\"${_codelet_fft_generator}\""
    "-DARGS=\"${_codelet_nofma_args}\""
    "-DRESULT_FILE=\"${_codelet_tmp_nofma_file}\"" "-P" "${FFTW_fftgen_rungenfft_script}"
    WORKING_DIRECTORY ${_codelet_binary_dir}
    DEPENDS ${fftgen_cmake_script} ${_codelet_fft_generator} ${FFTW_fftgen_rungenfft_script})

    add_custom_command(OUTPUT ${_codelet_tmp_fma_file}
    COMMAND ${CMAKE_COMMAND}
    "-DFFT_GEN=\"${_codelet_fft_generator}\""
    "-DARGS=\"${_codelet_fma_args}\""
    "-DRESULT_FILE=\"${_codelet_tmp_fma_file}\"" "-P" "${FFTW_fftgen_rungenfft_script}"
    WORKING_DIRECTORY ${_codelet_binary_dir}
    DEPENDS ${fftgen_cmake_script} ${_codelet_fft_generator} ${FFTW_fftgen_rungenfft_script})

    set(_codelet_codelet_output ${_codelet_source_dir}/${_codelet_codelet_source})
    if(ClangFormat_EXECUTABLE)
      set(_codelet_format_cmd COMMAND ${ClangFormat_EXECUTABLE} "-i" "${_codelet_codelet_output}" )
    endif()

    add_custom_command(OUTPUT ${_codelet_source_dir}/${_codelet_codelet_source}
    COMMAND ${CMAKE_COMMAND}
    "-DFMAFILE=\"${_codelet_tmp_fma_file}\""
    "-DSTDFILE=\"${_codelet_tmp_nofma_file}\""
    "-DPREFILE=\"${_codelet_prelude_file}\""
    "-DCPYFILE=\"${FFTW_copyright_header}\""
    "-DOUTFILE=\"${_codelet_codelet_output}\""
    "-P" ${FFTW_fftgen_fuse_script}
    ${_codelet_format_cmd}
    #COMMAND ${CMAKE_COMMAND} -E remove -f ${fma_file}
    #COMMAND ${CMAKE_COMMAND} -E remove -f ${non_fma_file}
    DEPENDS
    ${fftgen_fuse_script} ${_codelet_tmp_nofma_file} ${_codelet_tmp_fma_file} ${FFTW_fftgen_fuse_script}
    WORKING_DIRECTORY
    ${_codelet_binary_dir})
  endforeach()
endfunction()
