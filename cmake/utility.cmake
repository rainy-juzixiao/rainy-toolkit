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
include(CheckCXXSourceCompiles)

function(RAINY_GET_CXX_COMPILER_ID result_var)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(${result_var} "GCC" PARENT_SCOPE)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if(MSVC)
            set(${result_var} "MSVC-Clang" PARENT_SCOPE) # Clang on MSVC
        else()
            set(${result_var} "Clang" PARENT_SCOPE)      # 普通 Clang
        endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set(${result_var} "MSVC" PARENT_SCOPE)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "IntelLLVM")
        set(${result_var} "IntelLLVM" PARENT_SCOPE)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
        set(${result_var} "Intel" PARENT_SCOPE)
    else()
        set(${result_var} "Unknown" PARENT_SCOPE)
    endif()
endfunction()

# 查找特定目录下的文件
# 递归查找特定后缀文件的函数
# 参数:
#   DIRECTORY - 要搜索的目录
#   EXTENSION - 要查找的文件扩展名（如 ".cpp"）
#   RESULT_VAR - 存储结果的变量名
function(rainy_load_flodar_files DIRECTORY EXTENSION RESULT_VAR)
    set(IS_LINUX FALSE)
    check_cxx_source_compiles("
    #if defined(_M_ARM64) || defined(__aarch64__)
    #error ARM64 not supported
    #endif
    int main() { return 0; }
    " IS_ARM64)
    if(IS_ARM64)
        set(IS_ARM64 false)
    else()
        set(IS_ARM64 true)
    endif()
    # 初始化结果列表
    set(FILE_LIST "")
    # 获取当前目录下的所有条目
    file(GLOB ENTRIES CONFIGURE_DEPENDS "${DIRECTORY}/*")
    # 遍历所有条目
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)$")
        set(IS_ARM64 TRUE)
    endif()
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(IS_LINUX TRUE)
    endif()
    foreach(ENTRY ${ENTRIES})
        # 初始化排除标志
        set(EXCLUDE_ENTRY FALSE)
        # 检查Linux平台排除条件
        if (IS_LINUX)
            if("${ENTRY}" MATCHES ".*[/\\\\]win[/\\\\].*")
                set(EXCLUDE_ENTRY TRUE)
            endif()
        # 检查Windows平台排除条件
        elseif(WIN32)
            if("${ENTRY}" MATCHES ".*[/\\\\]linux[/\\\\].*")
                set(EXCLUDE_ENTRY TRUE)
            endif()
        endif()
        if(IS_ARM64)
            if("${ENTRY}" MATCHES ".*[/\\\\]x86_64[/\\\\].*")
                set(EXCLUDE_ENTRY TRUE)
            endif()
        else()
            if("${ENTRY}" MATCHES ".*[/\\\\]arm64[/\\\\].*")
                set(EXCLUDE_ENTRY TRUE)
            endif()
        endif()
        # 处理非排除的条目
        if(NOT EXCLUDE_ENTRY)
            if(IS_DIRECTORY ${ENTRY})
                # 递归处理子目录
                unset(SUBDIR_RESULT)
                rainy_load_flodar_files(${ENTRY} ${EXTENSION} SUBDIR_RESULT)
                list(APPEND FILE_LIST ${SUBDIR_RESULT})
            else()
                # 提取文件扩展名
                get_filename_component(FILE_EXT ${ENTRY} EXT)
                # 确保扩展名匹配
                if(FILE_EXT STREQUAL "${EXTENSION}")
                    list(APPEND FILE_LIST ${ENTRY})
                endif()
            endif()
        endif()
    endforeach()
    # 将结果设置到父作用域
    set(${RESULT_VAR} ${FILE_LIST} PARENT_SCOPE)
endfunction()