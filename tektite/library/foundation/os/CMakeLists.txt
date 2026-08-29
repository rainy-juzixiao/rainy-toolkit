project(rainy-toolkit-foundation-os)

rainy_configure_target(rainy-toolkit-foundation-os)

target_include_directories(rainy-toolkit-foundation-os PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/sources" ".cxx" OS_SOURCES)
target_sources(rainy-toolkit-foundation-os PRIVATE ${OS_SOURCES})

target_link_libraries(rainy-toolkit-foundation-os PUBLIC rainy-toolkit-core)

set(RAINY_FOUNDATION_OS_TEST_FILES "")

rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/tests" ".cc" RAINY_FOUNDATION_OS_TEST_FILES)

add_executable(rainy-toolkit-foundation-os-unit-test ${RAINY_FOUNDATION_OS_TEST_FILES})

target_link_libraries(rainy-toolkit-foundation-os-unit-test PRIVATE Catch2::Catch2WithMain rainy-toolkit-foundation-os)

catch_discover_tests(rainy-toolkit-foundation-os-unit-test)

set_target_properties(rainy-toolkit-foundation-os-unit-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

unset(RAINY_FOUNDATION_OS_TEST_FILES)
unset(OS_SOURCES)

install(TARGETS rainy-toolkit-foundation-os EXPORT rainy-toolkit-targets
        RUNTIME DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/bin
        LIBRARY DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib
        ARCHIVE DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib)

install(DIRECTORY include/ DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/include)
