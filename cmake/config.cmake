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

rainy_load_flodar_files("${PROJECT_SOURCE_DIR}/xaga/sources" ".cxx" SPECIAL_FILES_LIST)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(rainy_toolkit_libraryname "rainy-toolkit-debug-package")
else()
    set(rainy_toolkit_libraryname "rainy-toolkit-release-package")
endif()

if (RAINY_BUILD_WITH_DYNAMIC)
    message("Build dynamic library target")
    add_library(rainy-toolkit SHARED ${SPECIAL_FILES_LIST})
    set_target_properties(rainy-toolkit PROPERTIES OUTPUT_NAME ${rainy_toolkit_libraryname})
    target_compile_definitions(rainy-toolkit PRIVATE RAINY_DYNAMIC_EXPORTS=1)
    target_compile_definitions(rainy-toolkit PUBLIC RAINY_USING_DYNAMIC=1)
else()
    message("Building library target")
    add_library(rainy-toolkit STATIC ${SPECIAL_FILES_LIST})
    target_compile_definitions(rainy-toolkit PRIVATE RAINY_DYNAMIC_EXPORTS=0)
    target_compile_definitions(rainy-toolkit PUBLIC RAINY_USING_DYNAMIC=0)
endif()

set_target_properties(rainy-toolkit PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_definitions(
    -DRAINY_TOOLKIT_PROJECT_VERSION="${PROJECT_VERSION}"
    -DRAINY_TOOLKIT_PROJECT_MAJOR=${PROJECT_VERSION_MAJOR}
    -DRAINY_TOOLKIT_PROJECT_MINOR=${PROJECT_VERSION_MINOR}
    -DRAINY_TOOLKIT_PROJECT_PATCH=${PROJECT_VERSION_PATCH}
)

target_include_directories(
    rainy-toolkit PUBLIC
    ${PROJECT_SOURCE_DIR}/xaga/include
)

message("Checking compiler...")
RAINY_GET_CXX_COMPILER_ID(COMPILER_ID)

message(STATUS "The rainy-toolkit will use ${COMPILER_ID} complier to compile the sources files")
message(STATUS "Starting configure the library")

if ((COMPILER_ID MATCHES "MSVC") OR (COMPILER_ID MATCHES "MSVC-Clang")) 
    if (RAINY_USE_AVX2_BOOST)
        message("The rainy-toolkit will using avx2 boost")
        target_compile_definitions(rainy-toolkit PUBLIC RAINY_USING_AVX2=1)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
    else()
        target_compile_definitions(rainy-toolkit PUBLIC RAINY_USING_AVX2=0)
    endif()
    set(MY_VERSIONINFO_RC "${CMAKE_BINARY_DIR}/version.rc")
    configure_file("${PROJECT_SOURCE_DIR}/cmake/msvc/version_template.rc"
                   "${MY_VERSIONINFO_RC}")
    target_sources(rainy-toolkit PRIVATE "${MY_VERSIONINFO_RC}")
endif()

if (CMAKE_COMPILER_IS_GNUCXX OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT MSVC))
    message("Detect Clang compiler or GNU compiler")
    if (RAINY_USE_AVX2_BOOST)
        message("The rainy-toolkit will using avx2 boost")
        add_definitions(-DRAINY_USING_AVX2=1)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
    else()
        add_definitions(-DRAINY_USING_AVX2=0)
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND MSVC)
    message("Detect Clang-MSVC Cli compiler")
    if (RAINY_USE_AVX2_BOOST)
        message("The rainy-toolkit will using avx2 boost")
        add_definitions(-DRAINY_USING_AVX2=1)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
    else()
        add_definitions(-DRAINY_USING_AVX2=0)
    endif()
endif ()

if (MSVC AND NOT (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    message("Detect MSVC compiler")
    if (RAINY_USE_AVX2_BOOST)
        message("The rainy-toolkit will using avx2 boost")
        add_definitions(-DRAINY_USING_AVX2=1)
        add_compile_options(/arch:AVX2)
    else()
        add_definitions(-DRAINY_USING_AVX2=0)
    endif() # RAINY_USE_AVX2_BOOST
    if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
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

if (COMPILER_ID MATCHES "MSVC") 
    target_compile_options(rainy-toolkit PRIVATE /W4 /w14996)
endif()

if (WIN32)
    message("Linking libraries for windows package")
	target_link_libraries(rainy-toolkit PRIVATE windowsapp)
	target_link_libraries(rainy-toolkit PRIVATE synchronization)
    target_link_libraries(rainy-toolkit PRIVATE dbghelp)
    target_link_libraries(rainy-toolkit PRIVATE dbgeng)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message("Linking libraries for linux package")
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()