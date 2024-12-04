cmake_minimum_required(VERSION 3.22.1)
project(rainy_toolkit)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message("Starting build rainy's toolkit")
message("Current C++ Standard Is ${CMAKE_CXX_STANDARD}")

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

# 添加头文件搜索路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

list(APPEND SOURCES 
    rainy/core/core.hpp
    rainy/core/core_format.hpp
    rainy/core/core_typetraits.hpp
    rainy/core/raw_stringview.hpp
)

list(APPEND SOURCES
    rainy/utility/invoke.hpp
    rainy/utility/iterator.hpp
    rainy/utility/ref_wrap.hpp
)

list(APPEND SOURCES
    rainy/diagnostics/contract.hpp
    rainy/diagnostics/exception_handler.hpp
    rainy/diagnostics/source_location.hpp
)

list(APPEND SOURCES
    rainy/user/hash/sha.hpp
    rainy/user/hash/hash.hpp
)

# 基础模块
list(APPEND SOURCES
        rainy/base.hpp
        rainy/utility.hpp
        rainy/algorithm.hpp
)

# 算法模块
list(APPEND SOURCES
        rainy/algorithm/container_operator.hpp
)

# 容器模块
list(APPEND SOURCES
        rainy/containers/any.hpp
        rainy/containers/optional.hpp
        rainy/containers/span.hpp
        rainy/containers/tuple.hpp
        rainy/containers/variant.hpp
        rainy/containers/vector.hpp
        rainy/containers/stack.hpp
)

# 文本模块
list(APPEND SOURCES
    rainy/text/format_wrapper.hpp
)

# 元编程
list(APPEND SOURCES
        rainy/meta/type_traits.hpp
        rainy/meta/reflection.hpp
        rainy/internals/reflection_core/field.hpp
        rainy/internals/reflection_core/method.hpp
        rainy/internals/reflection_core/abstract_type.hpp
        rainy/internals/reflection_core/method_invoker.hpp
)

# 异步支持
list(APPEND SOURCES
        rainy/user/async/coro_type_traits.hpp
        rainy/user/async/coroutine.hpp
        rainy/system/barrier_and_latch.hpp
)

# 内存库支持
list(APPEND SOURCES
        rainy/system/nebula_ptr.hpp
)

# 异常支持
list(APPEND SOURCES
        rainy/system/basic_exceptions.hpp
        rainy/system/exceptions.hpp
)

# 仿函数支持
list(APPEND SOURCES
        rainy/functional/function_pointer.hpp
)

# 针对 Windows 平台的特定源文件
if (WIN32)
    message("Adding Windows specific source files")
    list(APPEND SOURCES
            rainy/winapi/api_core.h
            rainy/winapi/environment.hpp
            rainy/winapi/injecter.hpp
            rainy/winapi/proc_thrd_mgr.hpp
            rainy/winapi/ui/window.hpp
    )
endif ()

# 设置测试用例
list(APPEND SOURCES test/main.cpp)

# 创建可执行文件
add_executable(rainy-toolkit ${SOURCES})

if (WIN32)
    # 为WindowsAPI链接cpp源文件（部分文件采用分离编译）
    target_sources(rainy-toolkit PRIVATE "rainy/sources/win/impl_thread.cxx")
endif ()

target_sources(rainy-toolkit PRIVATE "rainy/sources/sha/impl_sha.cxx")