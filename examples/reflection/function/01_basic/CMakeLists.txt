add_executable(rainy-toolkit-examples-reflection "main.cpp" "test.h")

set_target_properties(rainy-toolkit-examples-reflection PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
target_link_libraries(rainy-toolkit-examples-reflection PRIVATE rainy-toolkit)
add_library(test_dll SHARED test.cc)
target_link_libraries(test_dll PRIVATE rainy-toolkit)
set_target_properties(test_dll PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
target_link_libraries(rainy-toolkit-examples-reflection PRIVATE test_dll)

target_compile_options(rainy-toolkit-examples-reflection PRIVATE /source-charset:utf-8)
target_compile_options(rainy-toolkit-examples-reflection PRIVATE /execution-charset:GBK)
