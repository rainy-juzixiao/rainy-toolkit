project(rainy-toolkit-foundation-text)

add_rainy_interface_library(rainy-toolkit-foundation-text ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(rainy-toolkit-foundation-text INTERFACE rainy-toolkit-core)

set(RAINY_FOUNDATION_TEXT_TEST_FILES "")

rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/tests" ".cc" RAINY_FOUNDATION_TEXT_TEST_FILES)

add_executable(rainy-toolkit-foundation-text-unit-test ${RAINY_FOUNDATION_TEXT_TEST_FILES})

target_link_libraries(rainy-toolkit-foundation-text-unit-test PRIVATE Catch2::Catch2WithMain rainy-toolkit-foundation-text)

catch_discover_tests(rainy-toolkit-foundation-text-unit-test)

set_target_properties(rainy-toolkit-foundation-text-unit-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

unset(RAINY_FOUNDATION_TEXT_TEST_FILES)

install(TARGETS rainy-toolkit-foundation-text EXPORT rainy-toolkit-targets)

install(DIRECTORY include/ DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/include)
