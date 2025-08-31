add_executable(rainy-toolkit-benchmark-json
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)
target_link_libraries(rainy-toolkit-benchmark-json PUBLIC benchmark)
target_link_libraries(rainy-toolkit-benchmark-json PUBLIC rainy-toolkit)
target_include_directories(rainy-toolkit-benchmark-json PUBLIC rapidjson-1.1.0/rapidjson)

set_target_properties(rainy-toolkit-benchmark-json PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)