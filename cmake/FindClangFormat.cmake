# Find clang-format code formatter

include( FindPackageHandleStandardArgs )


find_program( ClangFormat_EXECUTABLE
  NAMES clang-format
)

if(ClangFormat_EXECUTABLE)
  execute_process(
    COMMAND ${ClangFormat_EXECUTABLE} "-version"
    OUTPUT_VARIABLE clangformat_version_string
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if(clangformat_version_string MATCHES "version ([0-9]+.[0-9]+.[0-9]+)")
    set(ClangFormat_VERSION ${CMAKE_MATCH_1})
  endif()
endif()


find_package_handle_standard_args( ClangFormat
  REQUIRED_VARS ClangFormat_EXECUTABLE
  VERSION_VAR ClangFormat_VERSION
)
