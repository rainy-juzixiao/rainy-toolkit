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

# 定义函数来查找特定目录下的文件（优化不在本库的工程考虑范围内，因为无意义）
# 递归查找特定后缀文件的函数
# 参数:
#   DIRECTORY - 要搜索的目录
#   EXTENSION - 要查找的文件扩展名（如 ".cpp"）
#   RESULT_VAR - 存储结果的变量名
function(rainy_load_flodar_files DIRECTORY EXTENSION RESULT_VAR)
    # 初始化结果列表
    set(FILE_LIST "")
    # 获取当前目录下的所有条目
    file(GLOB ENTRIES CONFIGURE_DEPENDS "${DIRECTORY}/*")
    # 因为使用GLOB_RECURSE并不是最佳实践，因此，拒绝使用
    # 遍历所有条目
    foreach(ENTRY ${ENTRIES})
        # 初始化排除标志
        set(EXCLUDE_ENTRY FALSE)
        # 检查Linux平台排除条件
        if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
            if("${ENTRY}" MATCHES ".*[/\\\\]winapi[/\\\\].*")
                set(EXCLUDE_ENTRY TRUE)
            endif()
        # 检查Windows平台排除条件
        elseif(WIN32)
            if("${ENTRY}" MATCHES ".*[/\\\\]linux_api[/\\\\].*")
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
