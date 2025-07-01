add_executable(rainy-toolkit-benchmark-ctti
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(rainy-toolkit-benchmark-ctti rainy-toolkit)
target_link_libraries(rainy-toolkit-benchmark-ctti benchmark)

set_target_properties(rainy-toolkit-benchmark-ctti PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
