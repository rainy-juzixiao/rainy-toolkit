cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message("rainy-utility: we're adding files from rainy/core/*")

set(RAINY_UTILITY_SOURCES
    invoke.hpp
    iterator.hpp
    ref_wrap.hpp
)

add_library(rainy_utility INTERFACE ${RAINY_CORE_SOURCES})
target_include_directories(rainy_utility INTERFACE ${CMAKE_SOURCE_DIR})

message("rainy-utility: done.")