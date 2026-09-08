project(rainy-toolkit-foundation-io-filesystem)

rainy_configure_target(rainy-toolkit-foundation-io-filesystem)

target_include_directories(rainy-toolkit-foundation-io-filesystem PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

set(FILESYSTEM_SOURCES "")
rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/sources" ".cxx" FILESYSTEM_SOURCES)
target_sources(rainy-toolkit-foundation-io-filesystem PRIVATE ${FILESYSTEM_SOURCES})

target_link_libraries(rainy-toolkit-foundation-io-filesystem PUBLIC rainy-toolkit-foundation-io-context)

set(RAINY_FOUNDATION_IO_FILESYSTEM_TEST_FILES "")
rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/tests" ".cc" RAINY_FOUNDATION_IO_FILESYSTEM_TEST_FILES)
add_executable(rainy-toolkit-foundation-io-filesystem-unit-test ${RAINY_FOUNDATION_IO_FILESYSTEM_TEST_FILES})
target_link_libraries(rainy-toolkit-foundation-io-filesystem-unit-test PRIVATE Catch2::Catch2WithMain rainy-toolkit-foundation-io-filesystem)
catch_discover_tests(rainy-toolkit-foundation-io-filesystem-unit-test)
set_target_properties(rainy-toolkit-foundation-io-filesystem-unit-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
unset(RAINY_FOUNDATION_IO_FILESYSTEM_TEST_FILES)

install(TARGETS rainy-toolkit-foundation-io-filesystem EXPORT rainy-toolkit-targets
        RUNTIME DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/bin
        LIBRARY DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib
        ARCHIVE DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib)

install(DIRECTORY include/ DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/include)

unset(FILESYSTEM_SOURCES)