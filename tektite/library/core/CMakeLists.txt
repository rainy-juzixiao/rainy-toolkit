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

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/platform)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/type_traits)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/annotations)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/container)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/diagnostics)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/algorithm)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/memory)

add_library(rainy-toolkit-core INTERFACE)

target_link_libraries(rainy-toolkit-core INTERFACE
        rainy-toolkit-core-platform
        rainy-toolkit-core-type_traits
        rainy-toolkit-core-annotation
        rainy-toolkit-core-container
        rainy-toolkit-core-diagnostics
        rainy-toolkit-core-algorithm
        rainy-toolkit-core-memory
)