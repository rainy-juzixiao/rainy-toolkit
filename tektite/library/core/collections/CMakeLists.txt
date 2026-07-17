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

project(rainy-toolkit-core-collections)

add_rainy_interface_library(rainy-toolkit-core-collections ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(rainy-toolkit-core-collections INTERFACE
        rainy-toolkit-core-platform
        rainy-toolkit-core-type_traits
        rainy-toolkit-core-algorithm
        rainy-toolkit-core-utility
        rainy-toolkit-core-memory
)

rainy_load_flodar_files("${PROJECT_SOURCE_DIR}/tests/" ".cc" FILES_LIST)
add_executable(rainy-toolkit-core-collections-unit-test ${FILES_LIST})
target_link_libraries(rainy-toolkit-core-collections-unit-test PRIVATE Catch2::Catch2WithMain rainy-toolkit-core-collections)