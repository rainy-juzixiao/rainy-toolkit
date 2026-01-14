#                                 Apache License
#                           Version 2.0, January 2004
#                        http://www.apache.org/licenses/
#
#   Copyright 2025 rainy-juzixiao
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

cmake_minimum_required(VERSION 3.22.1)
project(rainy-toolkit VERSION 0.7.4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(RAINY_TOOLKIT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

message("Starting build rainy's toolkit")
message("Current C++ Standard Is ${CMAKE_CXX_STANDARD}")

option(RAINY_USE_CROSSCOMPILE "Enable for cross compile" off)

option(RAINY_ENABLE_TESTING "Enable for testing." on)
option(RAINY_USE_AVX2_BOOST "Enable for avx2 boost." on)
option(RAINY_BUILD_WITH_DYNAMIC
    "This option allows you build rainy-toolkit as a dynamic libraray." on)
option(RAINY_USING_UTF8_INPUT_FOR_MSVC "Use utf-8 encoding for input. Reason: we need avoid mutilangauge problems." on)
option(RAINY_USING_UTF8_OUTPUT_FOR_MSVC "Use utf-8 encoding for output. Reason: we need avoid mutilangauge problems." on)

include(${PROJECT_SOURCE_DIR}/cmake/utility.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/config.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/install.cmake)

if (RAINY_ENABLE_TESTING)
    message("Testing is enable!")
    enable_testing()
    include(CTest)
    add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/catch2)
    add_subdirectory(${PROJECT_SOURCE_DIR}/xaga/tests)
endif()

set( BENCHMARK_ENABLE_TESTING off)

#add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/benchmark)
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/rttr_ex)

add_subdirectory(${PROJECT_SOURCE_DIR}/xaga/examples/reflection/)
#add_subdirectory(${PROJECT_SOURCE_DIR}/xaga/benchmark/any/)
#add_subdirectory(${PROJECT_SOURCE_DIR}/xaga/benchmark/reflection/)