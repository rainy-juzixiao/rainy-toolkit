cmake_minimum_required(VERSION 3.22.1)
project(rainy_toolkit)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message("Starting build rainy's toolkit")
message("Current C++ Standard Is ${CMAKE_CXX_STANDARD}")

option(RAINY_ENABLE_TESTING "Enable for testing." OFF)
option(MOUDLE_USE_SOURCE_BUILD_MODE "Use source build mode for all module" ON)
option(RAINY_USING_UTF8_INPUT_FOR_MSVC "Use utf-8 encoding for input. Reason: we need avoid mutilangauge problems." ON)
option(RAINY_USING_UTF8_OUTPUT_FOR_MSVC "Use utf-8 encoding for output. Reason: we need avoid mutilangauge problems." ON)

set(RAINY_ENABLE_TESTING on)
set(MOUDLE_USE_SOURCE_BUILD_MODE on)
set(RAINY_USING_UTF8_INPUT_FOR_MSVC on)
set(RAINY_USING_UTF8_OUTPUT_FOR_MSVC off)

if (MOUDLE_USE_SOURCE_BUILD_MODE)
    message("Build source mode target")
    add_library(rainy-toolkit)
else()
    message("Build pure-header target")
    add_library(rainy-toolkit INTERFACE)
endif()

if (MOUDLE_USE_SOURCE_BUILD_MODE)

target_include_directories(
    rainy-toolkit PUBLIC
    ${PROJECT_SOURCE_DIR}/master/include
)

else()

target_include_directories(
    rainy-toolkit INTERFACE
    ${PROJECT_SOURCE_DIR}/master/include
)

endif()

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message("Detect Clang compiler or GNU compiler")
    add_compile_options(-mavx2)
endif ()

if (MSVC AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message("Detect MSVC compiler")
    add_compile_options(/arch:AVX2)
    if (MOUDLE_USE_SOURCE_BUILD_MODE)
        if (RAINY_USING_UTF8_INPUT_FOR_MSVC)
            message("Using UTF-8 for input encoding.")
            target_compile_options(rainy-toolkit PUBLIC /source-charset:utf-8)
        else()
            message("Using GBK for input encoding.")
            target_compile_options(rainy-toolkit PUBLIC /execution-charset:gbk)
        endif()
        if (RAINY_USING_UTF8_OUTPUT_FOR_MSVC)
            message("Using UTF-8 for output encoding.")
            target_compile_options(rainy-toolkit PUBLIC /source-charset:utf-8)
        else()
            message("Using GBK for output encoding.")
            target_compile_options(rainy-toolkit PUBLIC /execution-charset:gbk)
        endif()
    endif()
endif ()

if (MOUDLE_USE_SOURCE_BUILD_MODE)
    message("MOUDLE_USE_SOURCE_BUILD_MODE is enabled!")
    add_definitions(-DRAINY_USING_SOURCESFILE_BUILD=1)
else()
    message("MOUDLE_USE_SOURCE_BUILD_MODE is disabled!")
    add_definitions(-DRAINY_USING_SOURCESFILE_BUILD=0)
endif()

if (NOT MOUDLE_USE_SOURCE_BUILD_MODE)
    target_include_directories(
        rainy-toolkit INTERFACE
        ${PROJECT_SOURCE_DIR}/master/sources
    )
endif()

include(${PROJECT_SOURCE_DIR}/cmake/linker/rainy_toolkit_target_link.cmake)

if (RAINY_ENABLE_TESTING)
    message("Testing is enable!")
    enable_testing()
    add_subdirectory(tests)
endif()