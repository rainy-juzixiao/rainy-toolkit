project(rainy-toolkit-foundation-factory)

rainy_configure_target(rainy-toolkit-foundation-factory)

target_include_directories(rainy-toolkit-foundation-factory PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/sources" ".cxx" FACTORY_SOURCES)
target_sources(rainy-toolkit-foundation-factory PRIVATE ${FACTORY_SOURCES})

target_link_libraries(rainy-toolkit-foundation-factory PUBLIC rainy-toolkit-core rainy-toolkit-foundation-concurrency)

set(RAINY_FOUNDATION_FACTORY_TEST_FILES "")

rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/tests" ".cc" RAINY_FOUNDATION_FACTORY_TEST_FILES)

add_executable(rainy-toolkit-foundation-factory-unit-test ${RAINY_FOUNDATION_FACTORY_TEST_FILES})

target_link_libraries(rainy-toolkit-foundation-factory-unit-test PRIVATE Catch2::Catch2WithMain rainy-toolkit-foundation-factory rainy-toolkit-core)

catch_discover_tests(rainy-toolkit-foundation-factory-unit-test)

set_target_properties(rainy-toolkit-foundation-factory-unit-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

unset(RAINY_FOUNDATION_FACTORY_TEST_FILES)
unset(FACTORY_SOURCES)

install(TARGETS rainy-toolkit-foundation-factory EXPORT rainy-toolkit-targets
        RUNTIME DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/bin
        LIBRARY DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib
        ARCHIVE DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib)

install(DIRECTORY include/ DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/include)
