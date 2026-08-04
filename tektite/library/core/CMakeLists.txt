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

project(rainy-toolkit-core)

rainy_configure_target(rainy-toolkit-core)

target_include_directories(rainy-toolkit-core PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/sources" ".cxx" SUBDIR_SOURCES)
target_sources(rainy-toolkit-core PRIVATE ${SUBDIR_SOURCES})

set(RAINY_TOOLKIT_CORE_TEST_FILES "")

rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/tests" ".cc" RAINY_TOOLKIT_CORE_TEST_FILES)

add_executable(rainy-toolkit-core-unit-test ${RAINY_TOOLKIT_CORE_TEST_FILES})

target_link_libraries(rainy-toolkit-core-unit-test PRIVATE Catch2::Catch2WithMain rainy-toolkit-core)

set_target_properties(rainy-toolkit-core-unit-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

unset(RAINY_TOOLKIT_CORE_TEST_FILES)
