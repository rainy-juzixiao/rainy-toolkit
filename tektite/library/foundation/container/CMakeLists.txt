project(rainy-toolkit-foundation-container)

rainy_configure_target(rainy-toolkit-foundation-container)

target_include_directories(rainy-toolkit-foundation-container PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/sources" ".cxx" CONTAINER_SOURCES)
target_sources(rainy-toolkit-foundation-container PRIVATE ${CONTAINER_SOURCES})

target_link_libraries(rainy-toolkit-foundation-container PUBLIC rainy-toolkit-core rainy-toolkit-foundation-concurrency)

set(RAINY_FOUNDATION_CONTAINER_TEST_FILES "")

rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/tests" ".cc" RAINY_FOUNDATION_CONTAINER_TEST_FILES)

add_executable(rainy-toolkit-foundation-container-unit-test ${RAINY_FOUNDATION_CONTAINER_TEST_FILES})

target_link_libraries(rainy-toolkit-foundation-container-unit-test PRIVATE Catch2::Catch2WithMain rainy-toolkit-foundation-container)

catch_discover_tests(rainy-toolkit-foundation-container-unit-test)

set_target_properties(rainy-toolkit-foundation-container-unit-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

unset(RAINY_FOUNDATION_CONTAINER_TEST_FILES)
unset(CONTAINER_SOURCES)

install(TARGETS rainy-toolkit-foundation-container EXPORT rainy-toolkit-targets
        RUNTIME DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/bin
        LIBRARY DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib
        ARCHIVE DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib)

install(DIRECTORY include/ DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/include)
