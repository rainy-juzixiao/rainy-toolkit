add_executable(rainy-toolkit-benchmark-event 
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(rainy-toolkit-benchmark-event rainy-toolkit)
target_link_libraries(rainy-toolkit-benchmark-event benchmark)
target_link_libraries(rainy-toolkit-benchmark-event EnTT)

target_include_directories(rainy-toolkit-benchmark-event PUBLIC ${PROJECT_SOURCE_DIR}/third_party/udrefl/include)

set_target_properties(rainy-toolkit-benchmark-event PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)