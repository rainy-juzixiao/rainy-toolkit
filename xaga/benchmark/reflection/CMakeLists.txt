add_executable(rainy-toolkit-benchmark-reflection 
	${CMAKE_CURRENT_SOURCE_DIR}/src/function.cc
	${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)


target_link_libraries(rainy-toolkit-benchmark-reflection rainy-toolkit)
target_link_libraries(rainy-toolkit-benchmark-reflection benchmark)
target_link_libraries(rainy-toolkit-benchmark-reflection Ubpa::UDRefl_core)
target_link_libraries(rainy-toolkit-benchmark-reflection meta)
target_link_libraries(rainy-toolkit-benchmark-reflection rttr_core)

target_include_directories(rainy-toolkit-benchmark-reflection PUBLIC ${PROJECT_SOURCE_DIR}/third_party/udrefl/include)

set_target_properties(rainy-toolkit-benchmark-reflection PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)