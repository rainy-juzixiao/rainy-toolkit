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
#ifndef RAINY_STREAM_PRINT_HPP
#define RAINY_STREAM_PRINT_HPP
#include <cstdio>
#include <mutex>
#include <rainy/core/core.hpp>
#include <shared_mutex>
#include <sstream>
#include <string>

namespace rainy::foundation::io::implements {
    RAINY_INLINE std::shared_mutex &get_mtx() {
        static std::shared_mutex lock;
        return lock;
    }

    template <typename... Args>
    std::string make_sstream_string(Args... args) {
        std::stringstream stream;
        ((stream << args), ...);
        return stream.str();
    }

    template <typename Ty, typename = void>
    RAINY_CONSTEXPR_BOOL is_printable_impl = false;

    template <typename Ty>
    RAINY_CONSTEXPR_BOOL is_printable_impl<
        Ty, type_traits::other_trans::void_t<decltype(utility::declval<std::ostream &>() << utility::declval<Ty>())>> = true;

    template <typename... Args>
    RAINY_CONSTEXPR_BOOL is_printable_v = (is_printable_impl<Args> && ...);
}

namespace rainy::foundation::io {
    /**
     * @brief 将任意参数打印到标准输出流
     * @tparam Args 打印参数类型
     * @param args 打印参数
     * @param 如果参数不可被被用于输出。则会导致编译错误。因为enable_if将检查其参数是否能用于输出
     * @param 要求如下：
     * @param object [Type=Any(Can be overloaded)]
     * @param ostream [Type=std::ostream]
     * @param ostream << object;
     */
    template <typename... Args, type_traits::other_trans::enable_if_t<implements::is_printable_v<Args...>, int> = 0>
    void stdout_print(Args... args) {
        std::string output_string = implements::make_sstream_string(args...);
        (void) std::fwrite(output_string.c_str(), sizeof(char), output_string.size(), stdout);
    }

    template <typename... Args, type_traits::other_trans::enable_if_t<implements::is_printable_v<Args...>, int> = 0>
    void stderr_print(Args... args) {
        std::string output_string = implements::make_sstream_string(args...);
        (void) std::fwrite(output_string.c_str(), sizeof(char), output_string.size(), stderr);
    }

    template <typename... Args, type_traits::other_trans::enable_if_t<implements::is_printable_v<Args...>, int> = 0>
    void stdout_println(Args... args) {
        std::string output_string = implements::make_sstream_string(args..., "\n");
        (void) std::fwrite(output_string.c_str(), sizeof(char), output_string.size(), stdout);
    }

    template <typename... Args, type_traits::other_trans::enable_if_t<implements::is_printable_v<Args...>, int> = 0>
    void stderr_println(Args... args) {
        std::string output_string = implements::make_sstream_string(args..., "\n");
        (void) std::fwrite(output_string.c_str(), sizeof(char), output_string.size(), stderr);
    }

    template <typename... Args, type_traits::other_trans::enable_if_t<implements::is_printable_v<Args...>, int> = 0>
    void ts_stdout_print(Args... args) {
        std::lock_guard<std::shared_mutex> lock(implements::get_mtx());
        stdout_print(args...);
    }

    template <typename... Args, type_traits::other_trans::enable_if_t<implements::is_printable_v<Args...>, int> = 0>
    void ts_stderr_print(Args... args) {
        std::lock_guard<std::shared_mutex> lock(implements::get_mtx());
        stderr_print(args...);
    }

    template <typename... Args, type_traits::other_trans::enable_if_t<implements::is_printable_v<Args...>, int> = 0>
    void ts_stdout_println(Args... args) {
        std::lock_guard<std::shared_mutex> lock(implements::get_mtx());
        stdout_println(args...);
    }

    template <typename... Args, type_traits::other_trans::enable_if_t<implements::is_printable_v<Args...>, int> = 0>
    void ts_stderr_println(Args... args) {
        std::lock_guard<std::shared_mutex> lock(implements::get_mtx());
        stderr_println(args...);
    }
}

#endif
