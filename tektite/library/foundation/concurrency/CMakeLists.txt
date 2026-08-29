project(rainy-toolkit-foundation-concurrency)

rainy_configure_target(rainy-toolkit-foundation-concurrency)

target_include_directories(rainy-toolkit-foundation-concurrency PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/sources" ".cxx" CONCURRENCY_SOURCES)
target_sources(rainy-toolkit-foundation-concurrency PRIVATE ${CONCURRENCY_SOURCES})

target_link_libraries(rainy-toolkit-foundation-concurrency PUBLIC rainy-toolkit-core)

set(RAINY_FOUNDATION_CONCURRENCY_TEST_FILES "")

rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/tests" ".cc" RAINY_FOUNDATION_CONCURRENCY_TEST_FILES)

add_executable(rainy-toolkit-foundation-concurrency-unit-test ${RAINY_FOUNDATION_CONCURRENCY_TEST_FILES})

target_link_libraries(rainy-toolkit-foundation-concurrency-unit-test PRIVATE Catch2::Catch2WithMain rainy-toolkit-foundation-concurrency)

catch_discover_tests(rainy-toolkit-foundation-concurrency-unit-test)

set_target_properties(rainy-toolkit-foundation-concurrency-unit-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

unset(RAINY_FOUNDATION_CONCURRENCY_TEST_FILES)
unset(CONCURRENCY_SOURCES)

install(TARGETS rainy-toolkit-foundation-concurrency EXPORT rainy-toolkit-targets
        RUNTIME DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/bin
        LIBRARY DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib
        ARCHIVE DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib)

install(DIRECTORY include/ DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/include)
