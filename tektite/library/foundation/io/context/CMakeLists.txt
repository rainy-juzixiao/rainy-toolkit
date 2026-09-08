project(rainy-toolkit-foundation-io-context)

rainy_configure_target(rainy-toolkit-foundation-io-context)

target_include_directories(rainy-toolkit-foundation-io-context PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

set(CONTEXT_SOURCES "")
rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/sources" ".cxx" CONTEXT_SOURCES)
target_sources(rainy-toolkit-foundation-io-context PRIVATE ${CONTEXT_SOURCES})

target_link_libraries(rainy-toolkit-foundation-io-context PUBLIC
        rainy-toolkit-core
        rainy-toolkit-foundation-concurrency
        rainy-toolkit-foundation-collections
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(rainy-toolkit-foundation-io-context PRIVATE uring)
elseif (WIN32)
    target_link_libraries(rainy-toolkit-foundation-io-context PRIVATE ws2_32)
endif ()

set(RAINY_FOUNDATION_IO_CONTEXT_TEST_FILES "")
rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/tests" ".cc" RAINY_FOUNDATION_IO_CONTEXT_TEST_FILES)
add_executable(rainy-toolkit-foundation-io-context-unit-test ${RAINY_FOUNDATION_IO_CONTEXT_TEST_FILES})
target_link_libraries(rainy-toolkit-foundation-io-context-unit-test PRIVATE Catch2::Catch2WithMain rainy-toolkit-foundation-io-context)
catch_discover_tests(rainy-toolkit-foundation-io-context-unit-test)
set_target_properties(rainy-toolkit-foundation-io-context-unit-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
unset(RAINY_FOUNDATION_IO_CONTEXT_TEST_FILES)

install(TARGETS rainy-toolkit-foundation-io-context EXPORT rainy-toolkit-targets
        RUNTIME DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/bin
        LIBRARY DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib
        ARCHIVE DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib)

install(DIRECTORY include/ DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/include)

unset(CONTEXT_SOURCES)