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
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(${result_var} "GCC" PARENT_SCOPE)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if (MSVC)
            set(${result_var} "MSVC-Clang" PARENT_SCOPE) # Clang on MSVC
        else ()
            set(${result_var} "Clang" PARENT_SCOPE)      # 普通 Clang
        endif ()
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set(${result_var} "MSVC" PARENT_SCOPE)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "IntelLLVM")
        set(${result_var} "IntelLLVM" PARENT_SCOPE)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Intel")
        set(${result_var} "Intel" PARENT_SCOPE)
    else ()
        set(${result_var} "Unknown" PARENT_SCOPE)
    endif ()
endfunction()

# 查找特定目录下的文件
# 递归查找特定后缀文件的函数
# 参数:
#   DIRECTORY - 要搜索的目录
#   EXTENSION - 要查找的文件扩展名（如 ".cpp"）
#   RESULT_VAR - 存储结果的变量名
function(rainy_load_flodar_files DIRECTORY EXTENSION RESULT_VAR)
    set(IS_LINUX FALSE)
    set(IS_MACOS FALSE)
    set(IS_POSIX FALSE)
    check_cxx_source_compiles("
    #if defined(_M_ARM64) || defined(__aarch64__)
    #error ARM64 not supported
    #endif
    int main() { return 0; }
    " IS_ARM64)
    if (IS_ARM64)
        set(IS_ARM64 false)
    else ()
        set(IS_ARM64 true)
    endif ()
    # 初始化结果列表
    set(FILE_LIST "")
    # 获取当前目录下的所有条目
    file(GLOB ENTRIES CONFIGURE_DEPENDS "${DIRECTORY}/*")
    # 遍历所有条目
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)$")
        set(IS_ARM64 TRUE)
    endif ()
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(IS_LINUX TRUE)
    endif ()
    if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(IS_MACOS TRUE)
    endif ()
    if (IS_LINUX OR IS_MACOS)
        set(IS_POSIX TRUE)
    endif ()
    foreach (ENTRY ${ENTRIES})
        # 初始化排除标志
        set(EXCLUDE_ENTRY FALSE)
        # 检查Linux平台排除条件
        if (IS_POSIX)
            if ("${ENTRY}" MATCHES ".*[/\\\\]win[/\\\\].*")
                set(EXCLUDE_ENTRY TRUE)
            endif ()
            if (IS_LINUX)
                if ("${ENTRY}" MATCHES ".*[/\\\\]apple[/\\\\].*")
                    set(EXCLUDE_ENTRY TRUE)
                endif()
            else ()
                if ("${ENTRY}" MATCHES ".*[/\\\\]linux[/\\\\].*")
                    set(EXCLUDE_ENTRY TRUE)
                endif()
            endif ()
        elseif (WIN32)  # 检查Windows平台排除条件
            if ("${ENTRY}" MATCHES ".*[/\\\\]posix[/\\\\].*")
                set(EXCLUDE_ENTRY TRUE)
            endif ()
        endif ()
        if (IS_ARM64)
            if ("${ENTRY}" MATCHES ".*[/\\\\]x86_64[/\\\\].*")
                set(EXCLUDE_ENTRY TRUE)
            endif ()
        else ()
            if ("${ENTRY}" MATCHES ".*[/\\\\]arm64[/\\\\].*")
                set(EXCLUDE_ENTRY TRUE)
            endif ()
        endif ()
        # 处理非排除的条目
        if (NOT EXCLUDE_ENTRY)
            if (IS_DIRECTORY ${ENTRY})
                # 递归处理子目录
                unset(SUBDIR_RESULT)
                rainy_load_flodar_files(${ENTRY} ${EXTENSION} SUBDIR_RESULT)
                list(APPEND FILE_LIST ${SUBDIR_RESULT})
            else ()
                # 提取文件扩展名
                get_filename_component(FILE_EXT ${ENTRY} EXT)
                # 确保扩展名匹配
                if (FILE_EXT STREQUAL "${EXTENSION}")
                    list(APPEND FILE_LIST ${ENTRY})
                endif ()
            endif ()
        endif ()
    endforeach ()
    # 将结果设置到父作用域
    set(${RESULT_VAR} ${FILE_LIST} PARENT_SCOPE)
endfunction()

function(rainy_find_nodejs)
    if (NOT RAINY_USE_NODE_ADDON)
        message(WARNING "You need to enable RAINY_USE_NODE_ADDON then try reconfigure cmake again!")
        return()
    endif ()
    find_program(NODEJS_EXECUTABLE NAMES node nodejs)
    if (NOT NODEJS_EXECUTABLE)
        set(_nvm_search_hints "")
        if (UNIX)
            # nvm 默认路径
            set(_nvm_root "$ENV{NVM_DIR}")
            if ("${_nvm_root}" STREQUAL "")
                set(_nvm_root "$ENV{HOME}/.nvm")
            endif ()
            file(GLOB _nvm_node_bins
                    "${_nvm_root}/versions/node/*/bin"
            )
            list(APPEND _nvm_search_hints ${_nvm_node_bins})
            set(_fnm_root "$ENV{HOME}/.local/share/fnm")
            file(GLOB _fnm_node_bins
                    "${_fnm_root}/node-versions/*/installation/bin"
            )
            list(APPEND _nvm_search_hints ${_fnm_node_bins})
        elseif (WIN32)
            set(_nvm_root "$ENV{NVM_HOME}")
            if ("${_nvm_root}" STREQUAL "")
                set(_nvm_root "$ENV{APPDATA}/nvm")
            endif ()

            file(GLOB _nvm_node_bins
                    "${_nvm_root}/v*"
            )
            list(APPEND _nvm_search_hints ${_nvm_node_bins})

            # fnm on Windows
            file(GLOB _fnm_node_bins
                    "$ENV{LOCALAPPDATA}/fnm/node-versions/*/installation"
            )
            list(APPEND _nvm_search_hints ${_fnm_node_bins})
        endif ()

        find_program(NODEJS_EXECUTABLE
                NAMES node nodejs
                HINTS ${_nvm_search_hints}
                NO_DEFAULT_PATH
        )
    endif ()

    # 还找不到，报错并给出明确指引
    if (NOT NODEJS_EXECUTABLE)
        message(FATAL_ERROR
                "[rainy-toolkit] Node.js not found.\n"
                "NVM_DIR=$ENV{NVM_DIR}\n"
                "If you are using nvm, please specify manually:\n"
                "  cmake .. -DNODEJS_EXECUTABLE=$(which node)\n"
                "Or on Windows:\n"
                "  cmake .. -DNODEJS_EXECUTABLE=%NVM_HOME%\\vX.X.X\\node.exe"
        )
    endif ()

    message("[rainy-toolkit] Found Node.js: ${NODEJS_EXECUTABLE}")

    # 获取 Node.js 的根目录和可执行文件完整路径
    execute_process(
            COMMAND ${NODEJS_EXECUTABLE} -e "process.stdout.write(process.execPath)"
            OUTPUT_VARIABLE _node_exec
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # 保存 Node.js 可执行文件路径供后续使用
    set(RAINY_NODEJS_EXECUTABLE ${_node_exec} PARENT_SCOPE)

    cmake_path(SET _node_exec_path "${_node_exec}")
    cmake_path(GET _node_exec_path PARENT_PATH _node_bin_dir)
    cmake_path(GET _node_bin_dir PARENT_PATH _node_root)

    # 直接从 node_modules 里拿 headers
    execute_process(
            COMMAND ${NODEJS_EXECUTABLE} -e
            "try{const path=require('path');const pkg=require.resolve('node-api-headers/package.json');process.stdout.write(path.join(path.dirname(pkg),'include'))}catch(e){process.stdout.write('')}"
            WORKING_DIRECTORY ${RAINY_TOOLKIT_ROOT}
            OUTPUT_VARIABLE NODEJS_INCLUDE_DIR
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # 如果没有 node-api-headers，尝试从当前 Node.js runtime 获取
    if ("${NODEJS_INCLUDE_DIR}" STREQUAL "" OR NOT EXISTS "${NODEJS_INCLUDE_DIR}/node_api.h")
        cmake_path(APPEND _node_root "include" "node" OUTPUT_VARIABLE NODEJS_INCLUDE_DIR)
    endif ()

    # 还是找不到，报错并提示安装
    if (NOT EXISTS "${NODEJS_INCLUDE_DIR}/node_api.h")
        message(FATAL_ERROR
                "[rainy-toolkit] Cannot find Node.js headers.\n"
                "Please install node-api-headers:\n"
                "  npm install --save-dev node-api-headers\n"
                "\n"
                "Or ensure your Node.js installation includes headers."
        )
    endif ()

    execute_process(
            COMMAND ${NODEJS_EXECUTABLE} -e
            "try{process.stdout.write(require('node-addon-api').include.replace(/\"/g,''))}catch(e){process.stdout.write('')}"
            WORKING_DIRECTORY ${RAINY_TOOLKIT_ROOT}
            OUTPUT_VARIABLE NAPI_INCLUDE_DIR
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if ("${NAPI_INCLUDE_DIR}" STREQUAL "")
        message(FATAL_ERROR
                "[rainy-toolkit] node-addon-api not found.\n"
                "Please run: npm install --save-dev node-addon-api"
        )
    endif ()

    # Windows 特定处理
    if (WIN32)
        execute_process(
                COMMAND ${NODEJS_EXECUTABLE} -e "process.stdout.write(process.versions.node)"
                OUTPUT_VARIABLE NODE_VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        execute_process(
                COMMAND ${NODEJS_EXECUTABLE} -e "process.stdout.write(process.arch)"
                OUTPUT_VARIABLE NODE_ARCH
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        message("[rainy-toolkit] Node.js version: ${NODE_VERSION}, arch: ${NODE_ARCH}")
        set(_node_lib_paths "")
        # 1. node.exe 同级目录
        list(APPEND _node_lib_paths "${_node_bin_dir}/node.lib")
        # 2. node.exe 父目录下的 lib 子目录（某些安装结构）
        list(APPEND _node_lib_paths "${_node_root}/lib/node.lib")
        # 3. 常见 Program Files 安装路径
        if (DEFINED ENV{ProgramFiles})
            list(APPEND _node_lib_paths "$ENV{ProgramFiles}/nodejs/node.lib")
        endif ()
        if (DEFINED ENV{ProgramFiles\(x86\)})
            list(APPEND _node_lib_paths "$ENV{ProgramFiles\(x86\)}/nodejs/node.lib")
        endif ()
        # 4. nvm-windows / nvm4w 路径
        set(_nvm_root "$ENV{NVM_HOME}")
        if ("${_nvm_root}" STREQUAL "")
            set(_nvm_root "$ENV{APPDATA}/nvm")
        endif ()
        list(APPEND _node_lib_paths "${_nvm_root}/v${NODE_VERSION}/node.lib")
        list(APPEND _node_lib_paths "C:/nvm4w/nodejs/node.lib")   # 根据您的日志
        # 5. node-gyp 缓存目录
        set(_node_gyp_cache "$ENV{USERPROFILE}/.node-gyp/${NODE_VERSION}/${NODE_ARCH}")
        list(APPEND _node_lib_paths "${_node_gyp_cache}/node.lib")
        # 6. 从 PATH 中 node.exe 所在目录
        file(TO_CMAKE_PATH "$ENV{PATH}" _path_dirs)
        foreach (_path_dir ${_path_dirs})
            if (EXISTS "${_path_dir}/node.exe")
                list(APPEND _node_lib_paths "${_path_dir}/node.lib")
                cmake_path(GET _path_dir PARENT_PATH _parent_dir)
                list(APPEND _node_lib_paths "${_parent_dir}/lib/node.lib")
            endif ()
        endforeach ()
        set(RAINY_NODEJS_LIB "")
        foreach (_lib_path ${_node_lib_paths})
            if (EXISTS "${_lib_path}")
                set(RAINY_NODEJS_LIB "${_lib_path}")
                message("[rainy-toolkit] Found existing node.lib: ${_lib_path}")
                break()
            endif ()
        endforeach ()
        # --- 如果没找到，给出明确指引（不再自动下载）---
        if (NOT RAINY_NODEJS_LIB)
            message(FATAL_ERROR
                    "[rainy-toolkit] Cannot find node.lib for Node.js ${NODE_VERSION} (${NODE_ARCH}).\n"
                    "\n"
                    "Please ensure the Node.js development files are available. You can:\n"
                    "  1. Run this command to download the headers and lib:\n"
                    "       node-gyp install ${NODE_VERSION}\n"
                    "     (install node-gyp globally first if needed: npm install -g node-gyp)\n"
                    "  2. Manually download the files from https://nodejs.org/download/release/\n"
                    "     and place node.lib in one of these locations:\n"
                    "       ${_node_bin_dir}\n"
                    "       ${_node_gyp_cache}\n"
                    "  3. Set RAINY_NODEJS_LIB to the full path of node.lib in your CMake cache.\n"
                    "\n"
                    "After obtaining node.lib, re-run CMake."
            )
        endif ()
        set(RAINY_NODEJS_LIB ${RAINY_NODEJS_LIB} PARENT_SCOPE)
    endif ()
    message("[rainy-toolkit] Node.js headers : ${NODEJS_INCLUDE_DIR}")
    message("[rainy-toolkit] node-addon-api  : ${NAPI_INCLUDE_DIR}")
    set(RAINY_NODEJS_INCLUDE_DIR ${NODEJS_INCLUDE_DIR} PARENT_SCOPE)
    set(RAINY_NAPI_INCLUDE_DIR ${NAPI_INCLUDE_DIR} PARENT_SCOPE)
endfunction()

function(rainy_add_node_addon)
    cmake_parse_arguments(ARG "" "TARGET;OUTPUT_NAME" "SOURCES;INCLUDES;LINKS" ${ARGN})
    if (NOT ARG_TARGET)
        message(FATAL_ERROR "[rainy-toolkit] rainy_add_node_addon: TARGET is required")
    endif ()
    if (NOT ARG_OUTPUT_NAME)
        set(ARG_OUTPUT_NAME ${ARG_TARGET})
    endif ()
    add_library(${ARG_TARGET} SHARED ${ARG_SOURCES})
    set_target_properties(${ARG_TARGET} PROPERTIES
            PREFIX ""
            SUFFIX ".node"
            OUTPUT_NAME "${ARG_OUTPUT_NAME}"
            POSITION_INDEPENDENT_CODE ON
    )

    target_compile_definitions(${ARG_TARGET} PRIVATE
            NAPI_CPP_EXCEPTIONS
            NODE_ADDON_API_DISABLE_DEPRECATED
            BUILDING_NODE_EXTENSION
            NAPI_DLL
    )

    target_include_directories(${ARG_TARGET} PRIVATE
            ${RAINY_NODEJS_INCLUDE_DIR}
            ${RAINY_NAPI_INCLUDE_DIR}
            ${ARG_INCLUDES}
    )

    if (WIN32)
        if (NOT RAINY_NODEJS_LIB)
            message(FATAL_ERROR "[rainy-toolkit] RAINY_NODEJS_LIB not set. Did you call rainy_find_nodejs()?")
        endif ()

        message("[rainy-toolkit] Linking addon with: ${RAINY_NODEJS_LIB}")
        target_link_libraries(${ARG_TARGET} PRIVATE "${RAINY_NODEJS_LIB}")
        if (RAINY_NODEJS_DELAY_LOAD)
            target_link_options(${ARG_TARGET} PRIVATE /DELAYLOAD:${RAINY_NODEJS_DELAY_LOAD})
        endif ()
        target_link_libraries(${ARG_TARGET} PRIVATE
                delayimp.lib
                ${ARG_LINKS}
        )
        if (MSVC)
            set_target_properties(${ARG_TARGET} PROPERTIES
                    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
            )
        endif ()
    else ()
        target_link_options(${ARG_TARGET} PRIVATE "-Wl,--unresolved-symbols=ignore-all")
        target_link_libraries(${ARG_TARGET} PRIVATE ${ARG_LINKS})
    endif ()
    set_target_properties(${ARG_TARGET} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
    target_link_libraries(${ARG_TARGET} PRIVATE rainy-toolkit)
endfunction()