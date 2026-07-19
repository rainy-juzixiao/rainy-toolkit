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

if (RAINY_BUILD_WITH_DYNAMIC AND NOT RAINY_USE_CROSSCOMPILE)
    add_library(rainy-toolkit-core SHARED)
    target_compile_definitions(rainy-toolkit-core PRIVATE RAINY_DYNAMIC_EXPORTS=1)
    target_compile_definitions(rainy-toolkit-core PUBLIC RAINY_USING_DYNAMIC=1)
else ()
    add_library(rainy-toolkit-core STATIC)
    target_compile_definitions(rainy-toolkit-core PRIVATE RAINY_DYNAMIC_EXPORTS=0)
    target_compile_definitions(rainy-toolkit-core PUBLIC RAINY_USING_DYNAMIC=0)
endif ()

set_target_properties(rainy-toolkit-core PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" AND NOT RAINY_USE_CROSSCOMPILE)
    if (CMAKE_COMPILER_IS_GNUCXX OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT MSVC))
        if (RAINY_USE_AVX2_BOOST)
            add_definitions(-DRAINY_USING_AVX2=1)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
        else ()
            add_definitions(-DRAINY_USING_AVX2=0)
        endif ()
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND MSVC)
        if (RAINY_USE_AVX2_BOOST)
            add_definitions(-DRAINY_USING_AVX2=1)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
        else ()
            add_definitions(-DRAINY_USING_AVX2=0)
        endif ()
    endif ()
endif ()

if (MSVC AND NOT (CMAKE_CXX_COMPILER_ID MATCHES "Clang") AND NOT RAINY_USE_CROSSCOMPILE)
    if (RAINY_CAN_USE_AVX2)
        add_definitions(-DRAINY_USING_AVX2=1)
        add_compile_options(/arch:AVX2)
    else ()
        add_definitions(-DRAINY_USING_AVX2=0)
    endif ()
    if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if (RAINY_USING_UTF8_INPUT_FOR_MSVC)
            target_compile_options(rainy-toolkit-core PUBLIC /source-charset:utf-8)
        else ()
            target_compile_options(rainy-toolkit-core PUBLIC /execution-charset:gbk)
        endif ()
        if (RAINY_USING_UTF8_OUTPUT_FOR_MSVC)
            target_compile_options(rainy-toolkit-core PUBLIC /source-charset:utf-8)
        else ()
            target_compile_options(rainy-toolkit-core PUBLIC /execution-charset:gbk)
        endif ()
    endif ()
endif ()

set(RAINY_TOOLKIT_CORE_TEST_FILES "")

add_subdirectory(platform)
add_subdirectory(type_traits)
add_subdirectory(annotations)
add_subdirectory(algorithm)
add_subdirectory(utility)
add_subdirectory(exceptions)
add_subdirectory(container)
add_subdirectory(memory)
add_subdirectory(collections)
add_subdirectory(text)
add_subdirectory(diagnostics)

add_executable(rainy-toolkit-core-unit-test ${RAINY_TOOLKIT_CORE_TEST_FILES})
target_link_libraries(rainy-toolkit-core-unit-test PRIVATE Catch2::Catch2WithMain rainy-toolkit-core)

unset(RAINY_TOOLKIT_CORE_TEST_FILES)
