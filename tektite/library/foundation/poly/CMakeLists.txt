project(rainy-toolkit-foundation-poly)

rainy_configure_target(rainy-toolkit-foundation-poly)

target_include_directories(rainy-toolkit-foundation-poly PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/sources" ".cxx" POLY_SOURCES)
target_sources(rainy-toolkit-foundation-poly PRIVATE ${POLY_SOURCES})

target_link_libraries(rainy-toolkit-foundation-poly PUBLIC rainy-toolkit-core)

set(RAINY_FOUNDATION_POLY_TEST_FILES "")

rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/tests" ".cc" RAINY_FOUNDATION_POLY_TEST_FILES)

add_executable(rainy-toolkit-foundation-poly-unit-test ${RAINY_FOUNDATION_POLY_TEST_FILES})

target_link_libraries(rainy-toolkit-foundation-poly-unit-test PRIVATE Catch2::Catch2WithMain rainy-toolkit-foundation-poly)

set_target_properties(rainy-toolkit-foundation-poly-unit-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

unset(RAINY_FOUNDATION_POLY_TEST_FILES)
unset(POLY_SOURCES)
