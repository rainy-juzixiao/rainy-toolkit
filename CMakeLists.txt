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

option(RAINY_ENABLE_TESTING "Enable for testing." on)
option(RAINY_USE_AVX2_BOOST "Enable for avx2 boost." on)
option(RAINY_BUILD_WITH_DYNAMIC
        "This option allows you build rainy-toolkit as a dynamic libraray." on)
option(RAINY_USING_UTF8_INPUT_FOR_MSVC "Use utf-8 encoding for input. Reason: we need avoid mutilangauge problems." on)
option(RAINY_USING_UTF8_OUTPUT_FOR_MSVC "Use utf-8 encoding for output. Reason: we need avoid mutilangauge problems." on)
option(RAINY_USE_NODE_ADDON "Enable integration with node-addon-api (N-API)" off)
option(RAINY_USE_CXX26_REFLECTION_TS "Allows rainy-toolkit to use reflection ts of c++ 26 features for meta library" ON)
option(RAINY_USE_CROSSCOMPILE "Enable cross compilation" OFF)

set(RAINY_TOOLCHAIN_FILE "" CACHE FILEPATH "Path to CMake toolchain file for cross-compilation")

if(RAINY_USE_CROSSCOMPILE)
    if(NOT RAINY_TOOLCHAIN_FILE)
        message(FATAL_ERROR
            "RAINY_USE_CROSSCOMPILE is ON but RAINY_TOOLCHAIN_FILE is not set.\n"
            "Usage: cmake -DRAINY_USE_CROSSCOMPILE=ON -DRAINY_TOOLCHAIN_FILE=<path> ..")
    endif()
    if(NOT EXISTS "${RAINY_TOOLCHAIN_FILE}")
        message(FATAL_ERROR
            "RAINY_TOOLCHAIN_FILE '${RAINY_TOOLCHAIN_FILE}' not found.\n"
            "Please provide a valid toolchain file.")
    endif()
    message(STATUS "Cross-compilation enabled, using toolchain: ${RAINY_TOOLCHAIN_FILE}")
    set(CMAKE_TOOLCHAIN_FILE "${RAINY_TOOLCHAIN_FILE}" CACHE FILEPATH "Toolchain file" FORCE)
endif()

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" VERSION)
string(STRIP "${VERSION}" VERSION)

project(rainy-toolkit VERSION ${VERSION} LANGUAGES CXX C)

message(STATUS "Current rainy-toolkit version is ${VERSION}")

set(RAINY_TOOLKIT_CODE_NAME "xaga")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(RAINY_TOOLKIT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

message("Starting build rainy's toolkit")
message("Current C++ Standard Is ${CMAKE_CXX_STANDARD}")

include(${PROJECT_SOURCE_DIR}/cmake/utility.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/config.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/automoc.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/install.cmake)

if (RAINY_ENABLE_TESTING)
    message("Testing is enable!")
    enable_testing()
    include(CTest)
    add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/catch2)
#    add_subdirectory(${PROJECT_SOURCE_DIR}/xaga/tests)
endif ()

#add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/benchmark)
#add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/rttr_ex)

#add_subdirectory(${PROJECT_SOURCE_DIR}/xaga/examples/reflection/)
#add_subdirectory(${PROJECT_SOURCE_DIR}/xaga/benchmark/format/)
#add_subdirectory(${PROJECT_SOURCE_DIR}/xaga/benchmark/any/)
#add_subdirectory(${PROJECT_SOURCE_DIR}/xaga/benchmark/reflection/)

#if (RAINY_USE_NODE_ADDON)
#    add_subdirectory(${PROJECT_SOURCE_DIR}/xaga/internal_use/nodejs_integrate_test/)
#endif ()

add_subdirectory(tektite/)
