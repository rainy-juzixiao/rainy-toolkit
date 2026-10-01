project(rainy-toolkit-foundation-willow)

rainy_configure_target(rainy-toolkit-foundation-willow)

target_include_directories(rainy-toolkit-foundation-willow PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/sources" ".cxx" WILLOW_SOURCES)
target_sources(rainy-toolkit-foundation-willow PRIVATE ${WILLOW_SOURCES})

target_link_libraries(rainy-toolkit-foundation-willow PUBLIC
        rainy-toolkit-core
        rainy-toolkit-foundation-collections
)

set(RAINY_FOUNDATION_WILLOW_TEST_FILES "")

rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/tests" ".cc" RAINY_FOUNDATION_WILLOW_TEST_FILES)

add_executable(rainy-toolkit-foundation-willow-unit-test ${RAINY_FOUNDATION_WILLOW_TEST_FILES})

target_link_libraries(rainy-toolkit-foundation-willow-unit-test PRIVATE
        Catch2::Catch2WithMain
        rainy-toolkit-foundation-willow
)

catch_discover_tests(rainy-toolkit-foundation-willow-unit-test)

set_target_properties(rainy-toolkit-foundation-willow-unit-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

unset(RAINY_FOUNDATION_WILLOW_TEST_FILES)

set(RAINY_FOUNDATION_WILLOW_BENCHMARK_FILES "")

rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/benchmarks" ".cc" RAINY_FOUNDATION_WILLOW_BENCHMARK_FILES)

add_executable(rainy-toolkit-foundation-willow-benchmark ${RAINY_FOUNDATION_WILLOW_BENCHMARK_FILES})

target_link_libraries(rainy-toolkit-foundation-willow-benchmark PRIVATE
        benchmark::benchmark_main
        rainy-toolkit-foundation-willow
)

set_target_properties(rainy-toolkit-foundation-willow-benchmark PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

unset(RAINY_FOUNDATION_WILLOW_BENCHMARK_FILES)
unset(WILLOW_SOURCES)

install(TARGETS rainy-toolkit-foundation-willow EXPORT rainy-toolkit-targets
        RUNTIME DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/bin
        LIBRARY DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib
        ARCHIVE DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib)

install(DIRECTORY include/ DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/include)
