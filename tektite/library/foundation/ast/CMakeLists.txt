project(rainy-toolkit-foundation-ast)

add_rainy_interface_library(rainy-toolkit-foundation-ast ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(rainy-toolkit-foundation-ast INTERFACE rainy-toolkit-core)

set(RAINY_FOUNDATION_AST_TEST_FILES "")

rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/tests" ".cc" RAINY_FOUNDATION_AST_TEST_FILES)

add_executable(rainy-toolkit-foundation-ast-unit-test ${RAINY_FOUNDATION_AST_TEST_FILES})

target_link_libraries(rainy-toolkit-foundation-ast-unit-test PRIVATE Catch2::Catch2WithMain rainy-toolkit-foundation-ast)

catch_discover_tests(rainy-toolkit-foundation-ast-unit-test)

set_target_properties(rainy-toolkit-foundation-ast-unit-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

unset(RAINY_FOUNDATION_AST_TEST_FILES)

set(RAINY_FOUNDATION_AST_BENCHMARK_FILES "")

rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/benchmarks" ".cc" RAINY_FOUNDATION_AST_BENCHMARK_FILES)

add_executable(rainy-toolkit-foundation-ast-benchmark ${RAINY_FOUNDATION_AST_BENCHMARK_FILES})

target_link_libraries(rainy-toolkit-foundation-ast-benchmark PRIVATE benchmark::benchmark_main rainy-toolkit-foundation-ast)

set_target_properties(rainy-toolkit-foundation-ast-benchmark PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

unset(RAINY_FOUNDATION_AST_BENCHMARK_FILES)

install(TARGETS rainy-toolkit-foundation-ast EXPORT rainy-toolkit-targets)

install(DIRECTORY include/ DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/include)
