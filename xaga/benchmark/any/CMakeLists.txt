add_executable(rainy-toolkit-benchmark-any 
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(rainy-toolkit-benchmark-any rainy-toolkit)
target_link_libraries(rainy-toolkit-benchmark-any benchmark)
target_link_libraries(rainy-toolkit-benchmark-any EnTT)

set_target_properties(rainy-toolkit-benchmark-any PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
