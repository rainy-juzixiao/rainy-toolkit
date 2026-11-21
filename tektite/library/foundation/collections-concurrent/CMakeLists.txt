#                                 Apache License
#                           Version 2.0, January 2004
#                        http://www.apache.org/licenses/
#
#   Copyright 2026 rainy-juzixiao
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

project(rainy-toolkit-foundation-concurrent-collections)

add_rainy_interface_library(rainy-toolkit-foundation-concurrent-collections ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(rainy-toolkit-foundation-concurrent-collections INTERFACE rainy-toolkit-core)

if (RAINY_ENABLE_TESTING)
    set(RAINY_FOUNDATION_CONCURRENT_COLLECTIONS_TEST_FILES "")

    rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/tests" ".cc" RAINY_FOUNDATION_CONCURRENT_COLLECTIONS_TEST_FILES)

    add_executable(rainy-toolkit-foundation-concurrent-collections-unit-test ${RAINY_FOUNDATION_CONCURRENT_COLLECTIONS_TEST_FILES})

    target_link_libraries(rainy-toolkit-foundation-concurrent-collections-unit-test PRIVATE Catch2::Catch2WithMain rainy-toolkit-foundation-concurrent-collections rainy-toolkit-foundation-concurrency)

    catch_discover_tests(rainy-toolkit-foundation-concurrent-collections-unit-test)

    set_target_properties(rainy-toolkit-foundation-concurrent-collections-unit-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

    unset(RAINY_FOUNDATION_CONCURRENT_COLLECTIONS_TEST_FILES)
endif ()

install(TARGETS rainy-toolkit-foundation-concurrent-collections EXPORT rainy-toolkit-targets)

install(DIRECTORY include/ DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/include)
