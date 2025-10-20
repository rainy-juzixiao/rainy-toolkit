/*
 * Copyright 2025 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef RAINY_WINAPI_CORE_H
#define RAINY_WINAPI_CORE_H
#include <rainy/utility.hpp>
#include <windows.h>

#define RAINY_DECLARE_CHARSET_TEMPLATE                                                                                                \
    template <typename CharType = char,                                                                                               \
              rainy::type_traits::other_trans::enable_if_t<rainy::type_traits::type_relations::is_same_v<CharType, wchar_t> ||        \
                                                               rainy::type_traits::type_relations::is_same_v<CharType, char>,         \
                                                           int> = 0>

#define RAINY_TOOLKIT_WINAPI RAINY_TOOLKIT_API

namespace rainy::winapi::error_process {
    /**
     * @brief 用于获取Win32API调用的最后一个错误代码
     * @return Win32API调用的最后一个错误代码
     */
    RAINY_NODISCARD RAINY_INLINE unsigned long last_error() noexcept {
        return GetLastError();
    }

    /**
     * @brief 用于获取Win32API错误代码对应的字符串解释
     * @tparam CharType 字符集类型
     * @param error_code WIn32API错误码
     * @return 以std::string_view形式返回（需立即保存，避免同一线程再次调用导致的未定义行为）
     */
    RAINY_DECLARE_CHARSET_TEMPLATE RAINY_NODISCARD std::basic_string_view<CharType> last_error_message(
        unsigned long error_code = 0) noexcept {
        if (error_code == 0) {
            error_code = last_error();
        }
        constexpr std::size_t buffer_size = 768;
        thread_local CharType static_buffer[buffer_size]; // 768字节大小的缓冲区，应该足够装下了
#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 6387)
#endif
        if constexpr (type_traits::type_relations::is_same_v<CharType, wchar_t>) {
            unsigned long buffer_len =
                FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, error_code,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), static_cast<CharType>(static_buffer), buffer_size, nullptr);
            if (buffer_len == 0) {
                return  L"error in format_message";
            }
        } else {
            unsigned long buffer_len =
                FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, error_code,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), static_cast<CharType>(static_buffer), buffer_size, nullptr);
            if (buffer_len == 0) {
                return "error in format_message";
            }
        }
#if RAINY_USING_MSVC
#pragma warning(pop)
#endif
        return static_buffer;
    }
}

namespace rainy::winapi::dynamic_library {
    enum class get_module_errcode {
        ok,             // 成功
        file_not_found, // 找不到文件
        path_not_found, // 路径错误
        invalid_handle, // 句柄无效
        invalid_param,  // 参数无效
        unknown_error   // 未知错误
    };

    enum class load_library_errcode {
        ok,                 // 成功
        file_not_found,     // 找不到文件
        path_not_found,     // 路径错误
        bad_format,         // 格式错误
        invalid_access,     // 无效访问
        invalid_handle,     // 句柄无效
        mod_not_found,      // 找不到模块
        dll_init_failed,    // 初始化失败
        proc_not_found,     // 找不到过程
        not_enough_memory,  // 内存不足
        access_denied,      // 访问被拒绝
        invalid_param,      // 参数无效
        unknown_error       // 未知错误
    };
}

template <>
struct rainy::utility::pair<rainy::winapi::dynamic_library::get_module_errcode, std::uintptr_t> {
    using error_code_t = rainy::winapi::dynamic_library::get_module_errcode;
    using handle_t = std::uintptr_t;

    pair(error_code_t err_code, handle_t handle) : code(err_code), handle(handle) {
    }

    error_code_t code;
    handle_t handle;
};

template <>
struct rainy::utility::pair<rainy::winapi::dynamic_library::load_library_errcode, std::uintptr_t> {
    using error_code_t = rainy::winapi::dynamic_library::load_library_errcode;
    using handle_t = std::uintptr_t;

    pair(error_code_t err_code, handle_t handle) : code(err_code), handle(handle) {
    }

    error_code_t code;
    handle_t handle;
};

namespace rainy::winapi::dynamic_library {
    using get_module_ret_t = utility::pair<get_module_errcode, std::uintptr_t>;
    using load_library_ret_t = utility::pair<load_library_errcode, std::uintptr_t>;

    /**
     * @brief 从已经加载的模块中获取某个模块
     * @param module_name 模块名称
       @attention module_name的参数可以是.exe文件，也可以是.dll文件；但是必须是已加载的模块。如果留空会返回当前进程的句柄
       @attention 详细情况请参见 https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulehandlew
       @attention 另外，本函数被声明为noexcept。任何从此处引发的异常，将被认定是由Win32API调用异常引起的。请根据官方文档和相关API信息进行查询
     * @return 以pair形式返回错误码和模块。如果成功，错误码为ok，模块句柄为handle，否则错误码为相应错误码，模块句柄为nullptr
     */
    RAINY_DECLARE_CHARSET_TEMPLATE get_module_ret_t get_module(const CharType *module_name = nullptr) noexcept {
        get_module_errcode code = get_module_errcode::ok;
        HMODULE handle = nullptr;
        if constexpr (type_traits::helper::is_wchar_t<CharType>) {
            handle = GetModuleHandleW(module_name);
        } else {
            handle = GetModuleHandleA(module_name);
        }
        if (!handle) {
            switch (error_process::last_error()) {
                case ERROR_FILE_NOT_FOUND:
                    code = get_module_errcode::file_not_found;
                    break;
                case ERROR_PATH_NOT_FOUND:
                    code = get_module_errcode::path_not_found;
                    break;
                case ERROR_INVALID_HANDLE:
                    code = get_module_errcode::invalid_handle;
                    break;
                case ERROR_INVALID_PARAMETER:
                    code = get_module_errcode::invalid_param;
                    break;
                default:
                    code = get_module_errcode::unknown_error;
                    break;
            }
        }
        return get_module_ret_t{code, reinterpret_cast<std::uintptr_t>(handle)};
    }

    /**
     * @brief 从文件中加载某个二进制模块
     * @param library_name 模块名称
     * @attention library_name参数可以是.exe文件，也可以是.dll文件
     * @attention 详细情况请参见 https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibraryw
     * @attention 另外，本函数被声明为noexcept。任何从此处引发的异常，将被认定是由Win32API调用异常引起的。请根据官方文档和相关API信息进行查询
     * @return 以pair形式返回错误码和模块。如果成功，错误码为ok，模块句柄为handle，否则错误码为相应错误码，模块句柄为nullptr
     */
    RAINY_DECLARE_CHARSET_TEMPLATE load_library_ret_t load_library(const CharType *library_name = nullptr) noexcept {
        load_library_errcode code = load_library_errcode::ok;
        HMODULE handle = nullptr;
        if constexpr (type_traits::helper::is_wchar_t<CharType>) {
            handle = LoadLibraryW(library_name);
        } else {
            handle = LoadLibraryA(library_name);
        }
        if (!handle) {
            switch (error_process::last_error()) {
                case ERROR_FILE_NOT_FOUND:
                    code = load_library_errcode::file_not_found;
                    break;
                case ERROR_PATH_NOT_FOUND:
                    code = load_library_errcode::path_not_found;
                    break;
                case ERROR_BAD_FORMAT:
                    code = load_library_errcode::bad_format;
                    break;
                case ERROR_INVALID_ACCESS:
                    code = load_library_errcode::invalid_access;
                    break;
                case ERROR_INVALID_HANDLE:
                    code = load_library_errcode::invalid_handle;
                    break;
                case ERROR_MOD_NOT_FOUND:
                    code = load_library_errcode::mod_not_found;
                    break;
                case ERROR_DLL_INIT_FAILED:
                    code = load_library_errcode::dll_init_failed;
                    break;
                case ERROR_PROC_NOT_FOUND:
                    code = load_library_errcode::proc_not_found;
                    break;
                case ERROR_ACCESS_DENIED:
                    code = load_library_errcode::access_denied;
                    break;
                case ERROR_INVALID_PARAMETER:
                    code = load_library_errcode::invalid_param;
                    break;
                default:
                    code = load_library_errcode::unknown_error;
                    break;
            }
        }
        return {code, reinterpret_cast<std::uintptr_t>(handle)};
    }
}

namespace rainy::winapi::dynamic_library {
    enum class load_runtime_fn_errcode {
        ok,                     // 成功
        proc_not_found,         // 无法从导出表找到proc
        invalid_handle,         // 句柄无效
        access_denied,          // 拒绝访问
        bad_format,             // 格式错误
        load_library_failed,    // 加载库失败
        unknown_error           // 未知错误
    };
}

namespace rainy::winapi::dynamic_library::implements {
    /**
     * @brief 从一个二进制模块中，获取某个运行时函数的地址
     * @tparam FunctionPointerSignature 函数指针签名（将检查此形参是否为指针类型，仅支持指针类型被编译）
     * @param handle 二进制模块
     * @param name 二进制模块导出表的名称
     * @return 如果成功，返回函数指针；否则返回nullptr
     * @attention 详细情况请参见 https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getprocaddress
     */
    template <typename FunctionPointerSignature,
              type_traits::other_trans::enable_if_t<type_traits::primary_types::is_pointer_v<FunctionPointerSignature>, int> = 0>
    FunctionPointerSignature get_fn_addr(std::uintptr_t handle, const std::string_view name) noexcept {
        return static_cast<bool>(handle)
                   ? reinterpret_cast<FunctionPointerSignature>(GetProcAddress(reinterpret_cast<HMODULE>(handle), name.data())) // NOLINT
                   : nullptr;
    }

    /**
     * @brief
     * 从一个二进制模块中，获取某个运行时函数的地址吗，并将其赋值给指定的容器（可以是函数指针、std::function或其他支持函数指针赋值的容器）
     * @tparam Container
     * 容器类型（将检查此形参是否为支持函数指针赋值的类型，仅支持以下类型被编译：原始函数指针、任意支持函数指针赋值的容器（eg.
     * std::function,rainy.foundation::functional::function_pointer,etc.））
     * @tparam FunctionPointerSignature 函数指针签名（将检查此形参是否为指针类型，仅支持指针类型被编译）
     * @param handle 二进制模块
     * @param function_name 函数名称
     * @param container 容器
     * @return 如果成功，返回ok，否则返回相应错误码
     * @attention 详细情况请参见 https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getprocaddress
     */
    template <typename FunctionPointerSignature, typename Container,
        type_traits::other_trans::enable_if_t<std::is_assignable_v<Container, FunctionPointerSignature> || type_traits::type_relations::is_convertible_v<Container, FunctionPointerSignature>, int> = 0>
    load_runtime_fn_errcode load_runtime_fn_proxy(std::uintptr_t handle, const std::string_view function_name,
                                                  Container &container) noexcept {
        if (!handle) {
            return load_runtime_fn_errcode::invalid_handle;
        }
        auto address = get_fn_addr<FunctionPointerSignature>(handle, function_name);
        if (!address) {
            switch (error_process::last_error()) {
                case ERROR_PROC_NOT_FOUND:
                    return load_runtime_fn_errcode::proc_not_found;
                case ERROR_INVALID_HANDLE:
                    return load_runtime_fn_errcode::invalid_handle;
                case ERROR_ACCESS_DENIED:
                    return load_runtime_fn_errcode::access_denied;
                case ERROR_BAD_FORMAT:
                    return load_runtime_fn_errcode::bad_format;
                default:
                    return load_runtime_fn_errcode::unknown_error;
            }
        }
        container = address;
        return load_runtime_fn_errcode::ok;
    }

    /// @brief 从一个二进制模块中，获取某个运行时函数的地址，并将其赋值给指定的容器（可以是函数指针、std::function或其他支持函数指针赋值的容器）
    /// @tparam FunctionPointerSignature 函数指针签名（将检查此形参是否为指针类型，仅支持指针类型被编译）
    /// @tparam Container 容器
    /// @tparam  
    /// @param library_name 
    /// @param function_name 
    /// @param container 
    /// @return 
    template <typename FunctionPointerSignature, typename Container,
              type_traits::other_trans::enable_if_t<
        std::is_assignable_v<Container, FunctionPointerSignature> || type_traits::type_relations::is_convertible_v<Container,FunctionPointerSignature>, int> = 0>
    load_runtime_fn_errcode load_runtime_fn_proxy(const std::string_view library_name, const std::string_view function_name,
                                                  Container &container) noexcept {
        std::uintptr_t dynamic_library = get_module(library_name.data()).handle; // NOLINT
        if (!dynamic_library) {
            dynamic_library = load_library(library_name.data()).handle; // NOLINT
            if (!dynamic_library) {
                return load_runtime_fn_errcode::load_library_failed;
            }
        }
        return load_runtime_fn_proxy<FunctionPointerSignature, Container>(dynamic_library, function_name, container);
    }
}

namespace rainy::winapi::dynamic_library {
    /**
     * @brief 从一个二进制模块中，获取某个运行时函数的地址，并将其赋值给指定的函数指针
     * @tparam FunctionPointer 函数指针类型（将检查此形参是否为指针类型，仅支持指针类型被编译）
     * @param handle 二进制模块
     * @param function_name 函数名称
     * @param function_ptr 函数指针的引用，用于接收函数指针
     * @return 如果成功返回ok，否则返回相应错误码
     */
    template <typename FunctionPointer,type_traits::other_trans::enable_if_t<type_traits::primary_types::is_pointer_v<FunctionPointer>,int> = 0>
    load_runtime_fn_errcode load_runtime_fn(std::uintptr_t handle, const std::string_view function_name, FunctionPointer& function_ptr) noexcept {
        return implements::load_runtime_fn_proxy<FunctionPointer>(handle, function_name, function_ptr);
    }

    /**
     * @brief 从一个二进制模块中，获取某个运行时函数的地址，并将其赋值给指定的function_pointer
     * @tparam Rx function_pointer的返回值类型
     * @tparam ...Args function_pointer的参数列表
     * @param handle 二进制模块
     * @param function_name 函数名称
     * @param function_pointer function_pointer的引用，用于接收函数指针
     * @return 如果成功返回ok，否则返回相应错误码
     */
    template <typename Rx, typename... Args>
    load_runtime_fn_errcode load_runtime_fn(std::uintptr_t handle, const std::string_view function_name,
                                            foundation::functional::function_pointer<Rx(Args...)> &function_pointer) noexcept {
        return implements::load_runtime_fn_proxy<Rx (*)(Args...), decltype(function_pointer)>(handle, function_name, function_pointer);
    }

    /**
     * @brief 从一个二进制模块中，获取某个运行时函数的地址，并将其赋值给指定的std::function
     * @tparam Rx std::function的返回值类型
     * @tparam ...Args std::function的参数列表
     * @param handle 二进制模块
     * @param function_name 函数名称
     * @param function std::function的引用，用于接收函数指针
     * @return 如果成功返回ok，否则返回相应错误码
     */
    template <typename Rx, typename... Args>
    load_runtime_fn_errcode load_runtime_fn(std::uintptr_t handle, const std::string_view function_name,
                                            std::function<Rx(Args...)> &function) noexcept {
        return implements::load_runtime_fn_proxy<Rx (*)(Args...), decltype(function)>(handle, function_name, function);
    }

    /**
     * @brief 从一个二进制模块中，获取某个运行时函数的地址，并将其赋值给指定的函数指针
     * @tparam FunctionPointer 函数指针类型（将检查此形参是否为指针类型，仅支持指针类型被编译）
     * @param library_name 要获取函数的二进制模块名称
     * @param function_name 
     * @param function_ptr 
     * @return 
     */
    template <typename FunctionPointer,type_traits::other_trans::enable_if_t<
        type_traits::primary_types::is_pointer_v<FunctionPointer>,int> = 0>
    load_runtime_fn_errcode load_runtime_fn(const std::string_view library_name, const std::string_view function_name,
                                            FunctionPointer &function_ptr) noexcept {
        return implements::load_runtime_fn_proxy<FunctionPointer>(library_name, function_name, function_ptr);
    }

    template <typename Rx, typename... Args>
    load_runtime_fn_errcode load_runtime_fn(const std::string_view library_name, const std::string_view function_name,
                                            std::function<Rx(Args...)> &function_ptr) noexcept {
        return implements::load_runtime_fn_proxy<Rx (*)(Args...), decltype(function_ptr)>(library_name,
                                                                                         function_name, function_ptr);
    }

    template <typename Rx, typename... Args>
    load_runtime_fn_errcode load_runtime_fn(const std::string_view library_name, const std::string_view function_name,
                                            foundation::functional::function_pointer<Rx(Args...)> &function_ptr) noexcept {
        return implements::load_runtime_fn_proxy<Rx (*)(Args...), decltype(function_ptr)>(library_name, function_name, function_ptr);
    }
}

namespace rainy::winapi::system::multithread {
    enum class win32_thread_result : int {
        success,
        nomem,
        timed_out,
        busy,
        error
    };
}

#endif
