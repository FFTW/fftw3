

add_executable(fftw-wisdom fftw-wisdom.c $<TARGET_OBJECTS:fftw_tests_bench_objects> )

set_target_properties(fftw-wisdom PROPERTIES OUTPUT_NAME "fftw${fftw_precision_suffix}-wisdom")
target_include_directories(fftw-wisdom PRIVATE
  ${fftw_config_path}
  ${CMAKE_SOURCE_DIR}/libbench2
  ${CMAKE_SOURCE_DIR}/api
)

target_link_libraries(fftw-wisdom PRIVATE fftw3 bench2)

install(TARGETS fftw-wisdom
RUNTIME DESTINATION bin)
