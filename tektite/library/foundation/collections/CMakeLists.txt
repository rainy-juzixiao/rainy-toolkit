project(rainy-toolkit-foundation-collections)

add_rainy_interface_library(rainy-toolkit-foundation-collections ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(rainy-toolkit-foundation-collections INTERFACE rainy-toolkit-core)

set(RAINY_FOUNDATION_COLLECTIONS_TEST_FILES "")

rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/tests" ".cc" RAINY_FOUNDATION_COLLECTIONS_TEST_FILES)

add_executable(rainy-toolkit-foundation-collections-unit-test ${RAINY_FOUNDATION_COLLECTIONS_TEST_FILES})

target_link_libraries(rainy-toolkit-foundation-collections-unit-test PRIVATE Catch2::Catch2WithMain rainy-toolkit-foundation-collections)

catch_discover_tests(rainy-toolkit-foundation-collections-unit-test)

set_target_properties(rainy-toolkit-foundation-collections-unit-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

unset(RAINY_FOUNDATION_COLLECTIONS_TEST_FILES)

install(TARGETS rainy-toolkit-foundation-collections EXPORT rainy-toolkit-targets)

install(DIRECTORY include/ DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/include)