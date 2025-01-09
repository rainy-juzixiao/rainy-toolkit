cmake_minimum_required(VERSION 3.22.1)
project(rainy_toolkit)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message("Starting build rainy's toolkit")
message("Current C++ Standard Is ${CMAKE_CXX_STANDARD}")

option(COMMON_MOUDLE_USE_SOURCE_BUILD_MODE "Use source build mode for common module" ON)
set(COMMON_MOUDLE_USE_SOURCE_BUILD_MODE off)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message("Detect Clang compiler or GNU compiler")
    add_compile_options(-mavx2)
endif ()

if (MSVC)
    message("Detect MSVC compiler")
    add_compile_options(/arch:AVX2)
endif ()

message("Adding optimize option....")

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")
endif ()

if (MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
endif ()

list(APPEND RAINY_HEADER_FILES test/main.cpp)

message(${RAINY_HEADER_FILES})

add_executable(rainy-toolkit ${RAINY_HEADER_FILES})

target_include_directories(
    rainy-toolkit PUBLIC
    ${PROJECT_SOURCE_DIR}/include/
)

if (COMMON_MOUDLE_USE_SOURCE_BUILD_MODE)
    message("USE_SOURCE_BUILD_MODE is enabled!")
    add_definitions(-DRAINY_USING_COMMON_SOURCESFILE_BUILD=1)
else()
    message("USE_SOURCE_BUILD_MODE is disabled!")
    add_definitions(-DRAINY_USING_COMMON_SOURCESFILE_BUILD=0)
endif()

include(${PROJECT_SOURCE_DIR}/cmake/rainy_toolkit_target_link.cmake)