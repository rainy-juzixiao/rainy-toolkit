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
#ifndef RAINY_SOURCE_LOCATION_HPP
#define RAINY_SOURCE_LOCATION_HPP
#include <cstring>
#include <rainy/core/platform.hpp>
#include <string>

#if RAINY_HAS_CXX20
#include <format>
#include <source_location>
#endif

#if RAINY_USING_GCC
#include <rainy/core/gnu/source_location.hpp>
#endif

namespace rainy::foundation::diagnostics {
    /**
     * @brief 提供当前代码源文件位置的元数据，包括文件名、函数名、行号和列号。
     * source_location类用于在编译时或运行时捕获当前代码位置的元数据信息。
     * @remark
     * 该类通常用于调试、日志记录和错误报告，以便在运行时获得代码的具体位置。
     */
    class source_location {
    public:
        /**
         * @brief 默认构造函数，初始化为未知的源位置。
         * 默认构造函数将所有位置数据设置为默认值（空指针或零）。
         */
        constexpr source_location() noexcept : file_(nullptr), function_(nullptr), line_(0), column_(0) {
        }

        /**
         * @brief 构造函数，使用提供的文件名、行号、函数名和列号来初始化source_location`。
         * @param file 源文件名。
         * @param ln 源文件中的行号。
         * @param function 函数名。
         * @param col 列号（默认值为 0）。
         */
        constexpr source_location(const char *file, const unsigned int ln, const char *function, const unsigned int col = 0) noexcept :
            file_(file), function_(function), line_(ln), column_(col) {
        }

#if RAINY_HAS_CXX20
        /**
         * @brief 从std::source_location对象构造source_location（仅在 C++20 及更高版本中可用）。
         * @param loc 标准库中的std::source_location对象。
         */
        source_location(const std::source_location &loc) noexcept :
            file_(loc.file_name()), function_(loc.function_name()), line_(loc.line()), column_(loc.column()) {
        }
#endif
        /**
         * @brief 获取当前代码位置的source_location对象。
         * @param line 当前代码的行号，默认使用编译器内建宏__builtin_LINE()获取。
         * @param col 当前代码的列号，默认使用编译器内建宏__builtin_COLUMN()获取（GCC除外，GCC为0）。
         * @param file 当前代码的文件名，默认使用编译器内建宏__builtin_FILE()获取。
         * @param function 当前代码的函数名，默认使用编译器内建宏__builtin_FUNCTION()获取。
         *
         * @return 返回当前代码位置的source_location对象。
         */
        RAINY_NODISCARD static constexpr source_location current(const unsigned int line = __builtin_LINE(),
#if RAINY_USING_GCC
                                                                 const unsigned int col = gcc_detail_impl::get_column(),
#else
                                                                 const unsigned int col = __builtin_COLUMN(),
#endif
                                                                 const char *const file = __builtin_FILE(),
                                                                 const char *const function = __builtin_FUNCTION()) noexcept {
            source_location result;
            result.file_ = file;
            result.line_ = line;
            result.column_ = col;
            result.function_ = function;
            return result;
        }

        /**
         * @brief 获取源文件名。
         * @return 返回源文件名的指针。
         */
        RAINY_NODISCARD constexpr const char *file_name() const noexcept {
            return file_;
        }

        /**
         * @brief 获取函数名。
         * @return 返回函数名的指针。
         */
        RAINY_NODISCARD constexpr const char *function_name() const noexcept {
            return function_;
        }

        /**
         * @brief 获取源文件中的行号。
         * @return 返回源文件中的行号。
         */
        RAINY_NODISCARD constexpr unsigned int line() const noexcept {
            return line_;
        }

        /**
         * @brief 获取源文件中的列号
         * @return 返回源文件中的列号
         */
        RAINY_NODISCARD constexpr unsigned int column() const noexcept {
            return column_;
        }

        /**
         * @brief 将源位置信息转换为字符串表示形式
         * @return 返回包含文件名、行号、列号和函数名的字符串
         * @remark 如果行号为0，则返回表示未知源位置的字符串
         */
        RAINY_NODISCARD std::string to_string() const {
            const unsigned long ln = line();
            if (ln == 0) {
                return "(unknown source location)";
            }
            std::string r = file_name();
            char buffer[16];
            (void) std::snprintf(buffer, std::size(buffer), ":%lu", ln);
            r += buffer;
            if (const unsigned long co = column(); co) {
                (void) std::snprintf(buffer, std::size(buffer), ":%lu", co);
                r += buffer;
            }
            if (const char *fn = function_name(); *fn != 0) {
                r += " in function '";
                r += fn;
                r += '\'';
            }
            return r;
        }

        /**
         * @brief 判断两个source_location对象是否相等。
         * @param s1 第一个source_location对象。
         * @param s2 第二个source_location对象。
         * @return 如果两个对象的文件名、函数名、行号和列号都相同，则返回true；否则返回false。
         */
        friend bool operator==(const source_location &s1, const source_location &s2) noexcept {
            return std::strcmp(s1.file_, s2.file_) == 0 && std::strcmp(s1.function_, s2.function_) == 0 && s1.line_ == s2.line_ &&
                   s1.column_ == s2.column_;
        }

        /**
         * @brief 判断两个source_location对象是否不相等。
         *
         * @param s1 第一个source_location对象。
         * @param s2 第二个source_location对象。
         * @return 如果两个对象的文件名、函数名、行号或列号不同，则返回true`；否则返回false`。
         */
        friend bool operator!=(const source_location &s1, const source_location &s2) noexcept {
            return !(s1 == s2);
        }

    private:
        const char *file_;
        const char *function_;
        unsigned int line_;
        unsigned int column_;
    };

    /**
     * @brief 获取当前代码位置的source_location对象。
     * @param ret 可选的默认source_location对象，默认为source_location::current()。
     * @return 返回当前代码位置的source_location对象。
     */
    constexpr source_location current_location(const source_location &ret = source_location::current()) noexcept {
        return ret;
    }
}

namespace rainy::utility {
    using foundation::diagnostics::source_location;
}

#if RAINY_HAS_CXX20
template <>
class std::formatter<rainy::foundation::diagnostics::source_location, char> {
public:
    explicit formatter() noexcept = default;

    static auto parse(format_parse_context &ctx) noexcept {
        return ctx.begin();
    }

    RAINY_NODISCARD static auto format(const rainy::foundation::diagnostics::source_location &value, std::format_context fc) noexcept {
        return std::format_to(fc.out(), "{}", value.to_string());
    }
};
#endif

#endif