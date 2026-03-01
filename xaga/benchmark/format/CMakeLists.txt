add_executable(rainy-toolkit-benchmark-format
	${CMAKE_CURRENT_SOURCE_DIR}/src/format_benchmark.cc
	${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

target_link_libraries(rainy-toolkit-benchmark-format rainy-toolkit)
target_link_libraries(rainy-toolkit-benchmark-format benchmark)

set_target_properties(rainy-toolkit-benchmark-format PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)