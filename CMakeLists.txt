cmake_minimum_required(VERSION 3.22.1)
project(rainy-toolkit VERSION 0.7.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message("Starting build rainy's toolkit")
message("Current C++ Standard Is ${CMAKE_CXX_STANDARD}")

option(RAINY_ENABLE_TESTING "Enable for testing." on)
option(RAINY_USE_AVX2_BOOST "Enable for avx2 boost." on)
option(RAINY_BUILD_WITH_DYNAMIC
    "This option allows you build rainy-toolkit as a dynamic libraray." on)
option(RAINY_USING_UTF8_INPUT_FOR_MSVC "Use utf-8 encoding for input. Reason: we need avoid mutilangauge problems." on)
option(RAINY_USING_UTF8_OUTPUT_FOR_MSVC "Use utf-8 encoding for output. Reason: we need avoid mutilangauge problems." on)

SET(RAINY_BUILD_WITH_DYNAMIC on)

include(${PROJECT_SOURCE_DIR}/cmake/utility.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/config.cmake)

if (RAINY_ENABLE_TESTING)
    message("Testing is enable!")
    enable_testing()
    add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/googletest)
    add_subdirectory(${PROJECT_SOURCE_DIR}/xaga/tests)
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/udrefl)
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/benchmark)
add_subdirectory(${PROJECT_SOURCE_DIR}/xaga/examples/reflection/)
add_subdirectory(${PROJECT_SOURCE_DIR}/xaga/benchmark)
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/meta)