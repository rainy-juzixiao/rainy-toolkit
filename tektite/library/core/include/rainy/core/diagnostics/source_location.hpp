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
#ifndef RAINY_CORE_SOURCE_LOCATION_HPP
#define RAINY_CORE_SOURCE_LOCATION_HPP
#include <cstring>
#include <rainy/core/platform.hpp>
#include <string>

#if RAINY_HAS_CXX20
#include <format>
#include <source_location>
#endif

namespace rainy::core::diagnostics {
    /**
     * \lang english
     * @brief Provides metadata about the current source code location, including file name, function name, line and column numbers.
     *  The source_location class captures metadata of the current code location at compile time or at runtime.
     * @remark This class is typically used for debugging, logging, and error reporting to obtain the specific code location at runtime.
     *
     * \lang simp-chinese
     * @brief 提供当前代码源文件位置的元数据，包括文件名、函数名、行号和列号。
     *  source_location类用于在编译时或运行时捕获当前代码位置的元数据信息。
     * @remark 该类通常用于调试、日志记录和错误报告，以便在运行时获得代码的具体位置。
     */
    class source_location {
    public:
        /**
         * \lang english
         * @brief Default constructor, initializing an unknown source location.
         *  The default constructor sets all location data to default values (null pointers or zero).
         *
         * \lang simp-chinese
         * @brief 默认构造函数，初始化为未知的源位置。
         *  默认构造函数将所有位置数据设置为默认值（空指针或零）。
         */
        constexpr source_location() noexcept : file_(nullptr), function_(nullptr), line_(0), column_(0) {
        }

        /**
         * \lang english
         * @brief Constructs a source_location with the given file name, line number, function name, and column number.
         * @param file The source file name.
         * @param ln The line number in the source file.
         * @param function The function name.
         * @param col The column number (default is 0).
         *
         * \lang simp-chinese
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
         * \lang english
         * @brief Constructs a source_location from a std::source_location object (available only in C++20 and later).
         * @param loc The std::source_location object from the standard library.
         *
         * \lang simp-chinese
         * @brief 从std::source_location对象构造source_location（仅在 C++20 及更高版本中可用）。
         * @param loc 标准库中的std::source_location对象。
         */
        source_location(const std::source_location &loc) noexcept :
            file_(loc.file_name()), function_(loc.function_name()), line_(loc.line()), column_(loc.column()) {
        }
#endif
        /**
         * \lang english
         * @brief Obtains a source_location object for the current code location.
         * @param line The line number of the current code, defaulting to the compiler built-in macro __builtin_LINE().
         * @param col The column number of the current code, defaulting to the compiler built-in macro __builtin_COLUMN() (except GCC, which is 0).
         * @param file The file name of the current code, defaulting to the compiler built-in macro __builtin_FILE().
         * @param function The function name of the current code, defaulting to the compiler built-in macro __builtin_FUNCTION().
         *
         * @return A source_location object for the current code location.
         *
         * \lang simp-chinese
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
                                                                 const unsigned int col = 0,
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
         * \lang english
         * @brief Gets the source file name.
         * @return A pointer to the source file name.
         *
         * \lang simp-chinese
         * @brief 获取源文件名。
         * @return 返回源文件名的指针。
         */
        RAINY_NODISCARD constexpr const char *file_name() const noexcept {
            return file_;
        }

        /**
         * \lang english
         * @brief Gets the function name.
         * @return A pointer to the function name.
         *
         * \lang simp-chinese
         * @brief 获取函数名。
         * @return 返回函数名的指针。
         */
        RAINY_NODISCARD constexpr const char *function_name() const noexcept {
            return function_;
        }

        /**
         * \lang english
         * @brief Gets the line number in the source file.
         * @return The line number in the source file.
         *
         * \lang simp-chinese
         * @brief 获取源文件中的行号。
         * @return 返回源文件中的行号。
         */
        RAINY_NODISCARD constexpr unsigned int line() const noexcept {
            return line_;
        }

        /**
         * \lang english
         * @brief Gets the column number in the source file.
         * @return The column number in the source file.
         *
         * \lang simp-chinese
         * @brief 获取源文件中的列号
         * @return 返回源文件中的列号
         */
        RAINY_NODISCARD constexpr unsigned int column() const noexcept {
            return column_;
        }

        /**
         * \lang english
         * @brief Converts the source location information to a string representation.
         * @return A string containing the file name, line number, column number, and function name.
         * @remark If the line number is 0, a string representing an unknown source location is returned.
         *
         * \lang simp-chinese
         * @brief 将源位置信息转换为字符串表示形式
         * @return 返回包含文件名、行号、列号和函数名的字符串
         * @remark 如果行号为0，则返回表示未知源位置的字符串
         */
        RAINY_NODISCARD std::string to_string() const {
            const unsigned long ln = line();
            if (ln == 0 || empty()) {
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
            if (const char *fn = function_name(); fn != nullptr && *fn != 0) {
                r += " in function '";
                r += fn;
                r += '\'';
            }
            return r;
        }

        /**
         * \lang english
         * @brief Determines whether two source_location objects are equal.
         * @param s1 The first source_location object.
         * @param s2 The second source_location object.
         * @return true if the file names, function names, line numbers, and column numbers of the two objects are all identical; otherwise false.
         *
         * \lang simp-chinese
         * @brief 判断两个source_location对象是否相等。
         * @param s1 第一个source_location对象。
         * @param s2 第二个source_location对象。
         * @return 如果两个对象的文件名、函数名、行号和列号都相同，则返回true；否则返回false。
         */
        friend bool operator==(const source_location &s1, const source_location &s2) noexcept {
            if (s1.empty() && s2.empty()) {
                return true;
            }
            if (s1.empty() || s2.empty()) {
                return false;
            }
            return std::strcmp(s1.file_, s2.file_) == 0 && std::strcmp(s1.function_, s2.function_) == 0 && s1.line_ == s2.line_ &&
                   s1.column_ == s2.column_;
        }

        /**
         * \lang english
         * @brief Determines whether two source_location objects are not equal.
         *
         * @param s1 The first source_location object.
         * @param s2 The second source_location object.
         * @return true if the file names, function names, line numbers, or column numbers of the two objects differ; otherwise false.
         *
         * \lang simp-chinese
         * @brief 判断两个source_location对象是否不相等。
         *
         * @param s1 第一个source_location对象。
         * @param s2 第二个source_location对象。
         * @return 如果两个对象的文件名、函数名、行号或列号不同，则返回true`；否则返回false`。
         */
        friend bool operator!=(const source_location &s1, const source_location &s2) noexcept {
            if (s1.empty() && s2.empty()) {
                return false;
            }
            if (s1.empty() || s2.empty()) {
                return true;
            }
            return !(s1 == s2);
        }

        /**
         * \lang english
         * @brief Checks whether this source_location is empty (unknown).
         *
         * @return true if the source location has no file name, false otherwise.
         *
         * \lang simp-chinese
         * @brief 检查此source_location是否为空（未知）。
         *
         * @return 如果源位置没有文件名则为true，否则为false。
         */
        bool empty() const noexcept {
            return !file_;
        }

    private:
        const char *file_;
        const char *function_;
        unsigned int line_;
        unsigned int column_;
    };

    /**
     * \lang english
     * @brief Obtains a source_location object for the current code location.
     * @param ret An optional default source_location object, defaulting to source_location::current().
     * @return A source_location object for the current code location.
     *
     * \lang simp-chinese
     * @brief 获取当前代码位置的source_location对象。
     * @param ret 可选的默认source_location对象，默认为source_location::current()。
     * @return 返回当前代码位置的source_location对象。
     */
    constexpr source_location current_location(const source_location &ret = source_location::current()) noexcept {
        return ret;
    }
}

namespace rainy::utility {
    using core::diagnostics::source_location;
}

#endif
